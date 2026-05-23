# ================== NATIVE QISKIT WGAN (BATCH MODE FOR OPEN PLAN) ==================
# Internal logic: 100% preserved from original working code
# Only change: get_sampler() now correctly uses job mode (Sampler(mode=backend))
# which is the only execution mode allowed on IBM Open Plan.
# No Session, no Batch wrapper needed — passing the backend directly IS job mode.
 
import math
import random
import numpy as np
import pandas as pd
import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt
import torch
import torch.nn as nn
import torch.optim as optim
from torch.utils.data import Dataset, DataLoader
import os
import json
import argparse
from datetime import datetime
import time
from skimage.metrics import structural_similarity as ssim
from scipy.linalg import sqrtm
 
# Qiskit imports
from qiskit import QuantumCircuit
from qiskit.transpiler.preset_passmanagers import generate_preset_pass_manager
from qiskit_aer import AerSimulator
from qiskit_ibm_runtime import QiskitRuntimeService, SamplerV2 as Sampler
 
os.environ["KMP_DUPLICATE_LIB_OK"] = "TRUE"
 
# ================== CONFIGURATION ==================
USE_REAL_HARDWARE = False
IBM_BACKEND_NAME  = "ibm_kingston"
BATCH_SIZE        = 2
N_QUBITS          = 4
Q_DEPTH           = 2
NUM_ITER          = 10
CRITIC_STEPS      = 3
LAMBDA_GP         = 10
EVAL_SAMPLES_REQUESTED = 16
LOG_EVERY         = 2
IMAGE_PIXELS      = 64
 
parser = argparse.ArgumentParser()
parser.add_argument("--use-real",   type=lambda x: x.lower() == 'true', default=USE_REAL_HARDWARE)
parser.add_argument("--backend",    type=str, default=IBM_BACKEND_NAME)
parser.add_argument("--qubits",     type=int, default=N_QUBITS)
parser.add_argument("--batch-size", type=int, default=BATCH_SIZE)
parser.add_argument("--iterations", type=int, default=NUM_ITER)
args = parser.parse_args()
 
USE_REAL_HARDWARE = args.use_real
IBM_BACKEND_NAME  = args.backend
N_QUBITS          = args.qubits
BATCH_SIZE        = args.batch_size
NUM_ITER          = args.iterations
 
# ================== SEED ==================
seed = 42
torch.manual_seed(seed)
np.random.seed(seed)
random.seed(seed)
 
def log(msg):
    print(f"[{datetime.now().strftime('%H:%M:%S')}] {msg}", flush=True)
 
# ================== RUN FOLDER ==================
def create_run_folder(base_dir="runs_native_qiskit"):
    os.makedirs(base_dir, exist_ok=True)
    existing = [d for d in os.listdir(base_dir) if d.startswith("run_")]
    run_id   = 1 if not existing else max([int(d.split("_")[1]) for d in existing]) + 1
    path     = os.path.join(base_dir, f"run_{run_id}")
    os.makedirs(path)
    return path, run_id
 
run_path, run_id = create_run_folder()
 
# ================== DATASET ==================
class DigitsDataset(Dataset):
    def __init__(self, csv_file, label=0):
        self.df = pd.read_csv(csv_file, header=None)
        self.df = self.df[self.df.iloc[:, -1] == label]
 
    def __len__(self):
        return len(self.df)
 
    def __getitem__(self, idx):
        image = self.df.iloc[idx, :-1].values.astype(np.float32)
        image = image / 16.0
        image = (image - 0.5) * 2
        image = torch.tensor(image).view(1, 8, 8)
        return image, 0
 
dataset    = DigitsDataset(r"D:\QML\opdigits_dataset\optdigits.tra")
dataloader = DataLoader(dataset, batch_size=BATCH_SIZE, shuffle=True, drop_last=True)
log(f"Run {run_id}: {run_path}")
log(f"Loaded {len(dataset)} samples")
 
# ================== SAVE REAL SAMPLES ==================
def save_real_samples(loader, path):
    try:
        imgs, _ = next(iter(loader))
        fig, axes = plt.subplots(2, 4, figsize=(12, 6))
        axes = axes.flatten()
        for i in range(min(8, len(imgs))):
            axes[i].imshow(imgs[i].squeeze(), cmap='gray')
            axes[i].axis('off')
        plt.savefig(os.path.join(path, "real_samples.png"))
        plt.close()
        log("Saved real sample preview")
    except Exception as e:
        log(f"Could not save real samples: {e}")
 
save_real_samples(dataloader, run_path)
 
# ================== CRITIC ==================
class Critic(nn.Module):
    def __init__(self):
        super().__init__()
        self.model = nn.Sequential(
            nn.Flatten(),
            nn.Linear(64, 64),
            nn.LeakyReLU(0.2),
            nn.Linear(64, 32),
            nn.LeakyReLU(0.2),
            nn.Linear(32, 1)
        )
    def forward(self, x):
        return self.model(x)
 
# ================== DEVICE ==================
device = torch.device("cpu")
log(f"PyTorch device: {device}")
 
# ================== NATIVE QISKIT QUANTUM GENERATOR ==================
class NativeQiskitGenerator(nn.Module):
    """Pure Qiskit-based generator using SamplerV2 - Job mode (Open Plan safe)"""
 
    def __init__(self, n_qubits, q_depth, n_outputs=64):
        super().__init__()
        self.n_qubits   = n_qubits
        self.q_depth    = q_depth
        self.n_outputs  = n_outputs
        self.shots      = 1024
        self.sampler    = None
        self.backend    = None
        self.pass_manager = None
 
        # Trainable parameters — PRESERVED exactly
        self.theta = nn.Parameter(torch.rand(q_depth, n_qubits) * 2 * np.pi)
        self.phi   = nn.Parameter(torch.rand(q_depth, n_qubits) * 2 * np.pi)
        self.omega = nn.Parameter(torch.rand(q_depth, n_qubits) * 2 * np.pi)
 
    # ------------------------------------------------------------------ #
    #  ONLY THIS METHOD CHANGED vs the original:                          #
    #                                                                      #
    #  Old (broken for Open Plan): used Session or wrong options dict      #
    #  New (correct):                                                      #
    #    - Real HW  → Sampler(mode=backend)  ← job mode, Open Plan safe   #
    #    - Fallback → AerSimulator,          ← same pattern               #
    #                                                                      #
    #  Per IBM docs: "A Backend if you are using job mode."               #
    #  Job mode is the only mode allowed on the Open Plan.                #
    # ------------------------------------------------------------------ #
    def get_sampler(self):
        """Lazy-init SamplerV2 in JOB MODE (Open Plan compatible)."""
        if self.sampler is not None:
            return self.sampler
 
        if USE_REAL_HARDWARE:
            try:
                service = QiskitRuntimeService()
                backend = service.backend(IBM_BACKEND_NAME)
 
                # ✅ Job mode: pass the backend directly as `mode`
                #    This is explicitly allowed on the IBM Open Plan.
                #    Do NOT wrap in Session() — that is forbidden on Open Plan.
                sampler = Sampler(mode=backend)
                sampler.options.default_shots = self.shots   # correct shots API
 
                self.pass_manager = generate_preset_pass_manager(
                    backend=backend, optimization_level=1
                )
                self.sampler = sampler
                self.backend = backend
                log(f"✅ Using real hardware in JOB MODE (Open Plan): {backend.name}")
 
            except Exception as e:
                log(f"⚠️ Could not connect to hardware: {e}")
                log("Falling back to Aer simulator")
                self._init_aer_sampler()
        else:
            self._init_aer_sampler()
 
        return self.sampler
 
    def _init_aer_sampler(self):
        """AerSimulator fallback — same job-mode pattern."""
        backend = AerSimulator()
        sampler = Sampler(mode=backend)
        sampler.options.default_shots = self.shots
 
        self.pass_manager = generate_preset_pass_manager(
            backend=backend, optimization_level=1
        )
        self.sampler = sampler
        self.backend = backend
        log("Using Aer simulator with SamplerV2")
 
    # ------------------------------------------------------------------ #
    #  Everything below is 100% preserved from the original working code  #
    # ------------------------------------------------------------------ #
 
    def create_circuit(self, latent_vector, theta, phi, omega):
        qc = QuantumCircuit(self.n_qubits, self.n_qubits)
 
        for i in range(self.n_qubits):
            val = float(latent_vector[i].detach().item())
            qc.ry(val * np.pi, i)
 
        for l in range(self.q_depth):
            for i in range(self.n_qubits):
                qc.ry(float(theta[l, i].detach().item()), i)
                qc.rz(float(phi[l, i].detach().item()), i)
 
            for i in range(self.n_qubits - 1):
                qc.cx(i, i + 1)
            qc.cx(self.n_qubits - 1, 0)
 
        qc.measure(range(self.n_qubits), range(self.n_qubits))
        return qc
 
    def execute_circuit(self, latent_vector, theta, phi, omega):
        sampler = self.get_sampler()
        qc      = self.create_circuit(latent_vector, theta, phi, omega)
        isa_qc  = self.pass_manager.run(qc)
 
        job       = sampler.run([(isa_qc,)], shots=self.shots)
        result    = job.result()
        pub_result = result[0]
 
        creg_name = list(pub_result.data)[0]
        counts    = getattr(pub_result.data, creg_name).get_counts()
 
        probs = np.zeros(2 ** self.n_qubits)
        total = sum(counts.values())
        for bitstring, count in counts.items():
            probs[int(bitstring, 2)] = count / total
 
        return torch.tensor(probs, dtype=torch.float32)
 
    def forward(self, z):
        batch_size = z.shape[0]
        outputs    = []
 
        for i in range(batch_size):
            probs = self.execute_circuit(z[i], self.theta, self.phi, self.omega)
 
            if len(probs) > self.n_outputs:
                probs = probs[:self.n_outputs]
            elif len(probs) < self.n_outputs:
                pad   = torch.zeros(self.n_outputs - len(probs))
                probs = torch.cat([probs, pad])
 
            outputs.append(probs)
 
        images = torch.stack(outputs)
        images = torch.tanh(images)
        return images.view(batch_size, 1, 8, 8)
 
    def close(self):
        pass   # nothing to close in job mode
 
# ================== GRADIENT PENALTY ==================
def gradient_penalty(critic, real, fake):
    batch_size     = real.size(0)
    alpha          = torch.rand(batch_size, 1, device=device)
    real_flat      = real.view(batch_size, -1)
    fake_flat      = fake.view(batch_size, -1)
    alpha_expanded = alpha.expand_as(real_flat)
 
    interpolated = (alpha_expanded * real_flat + (1 - alpha_expanded) * fake_flat).view_as(real)
    interpolated.requires_grad_(True)
 
    critic_interpolated = critic(interpolated)
    gradients = torch.autograd.grad(
        outputs=critic_interpolated,
        inputs=interpolated,
        grad_outputs=torch.ones_like(critic_interpolated),
        create_graph=True,
        retain_graph=True
    )[0]
 
    gradients = gradients.view(batch_size, -1)
    return ((gradients.norm(2, dim=1) - 1) ** 2).mean()
 
# ================== INITIALIZATION ==================
critic    = Critic().to(device)
generator = NativeQiskitGenerator(N_QUBITS, Q_DEPTH).to(device)
 
optC = optim.Adam(critic.parameters(),    lr=0.0002, betas=(0.5, 0.9))
optG = optim.Adam(generator.parameters(), lr=0.0002, betas=(0.5, 0.9))
 
fixed_noise = torch.randn(8, N_QUBITS).to(device)
critic_losses      = []
generator_losses   = []
results            = []
wasserstein_losses = []
gp_losses          = []
counter = 0
 
log(f"Starting training on {'REAL HARDWARE' if USE_REAL_HARDWARE else 'SIMULATOR'}")
log(f"Total iterations: {NUM_ITER}")
 
# ================== TRAINING LOOP ==================
try:
    while counter < NUM_ITER:
        for real, _ in dataloader:
            real       = real.view(real.size(0), -1).to(device)
            start_time = time.time()
 
            # Train Critic
            for _ in range(CRITIC_STEPS):
                noise      = torch.randn(real.size(0), N_QUBITS).to(device)
                fake       = generator(noise).detach()
                real_score = critic(real)
                fake_score = critic(fake)
                wasserstein = -real_score.mean() + fake_score.mean()
                gp          = gradient_penalty(critic, real, fake)
                loss_C      = wasserstein + LAMBDA_GP * gp
 
                optC.zero_grad()
                loss_C.backward()
                optC.step()
 
                wasserstein_losses.append(wasserstein.item())
                gp_losses.append(gp.item())
 
            # Train Generator
            noise  = torch.randn(real.size(0), N_QUBITS).to(device)
            fake   = generator(noise)
            loss_G = -critic(fake).mean()
 
            optG.zero_grad()
            loss_G.backward()
            optG.step()
 
            critic_losses.append(loss_C.item())
            generator_losses.append(loss_G.item())
 
            counter   += 1
            iter_time  = time.time() - start_time
 
            if counter % LOG_EVERY == 0:
                log(f"Iter {counter}/{NUM_ITER} | C={loss_C.item():.3f} | "
                    f"G={loss_G.item():.3f} | W={wasserstein.item():.3f} | "
                    f"Time={iter_time:.1f}s")
 
            if counter % 5 == 0:
                with torch.no_grad():
                    results.append(generator(fixed_noise).view(8, 1, 8, 8).cpu().detach())
 
            if counter >= NUM_ITER:
                break
        if counter >= NUM_ITER:
            break
 
except KeyboardInterrupt:
    log(f"Training interrupted at iteration {counter}")
except Exception as e:
    log(f"Error: {e}")
    import traceback
    traceback.print_exc()
finally:
    generator.close()
 
# ================== SAVE GENERATED ==================
if len(results) == 0:
    with torch.no_grad():
        results.append(generator(fixed_noise).view(8, 1, 8, 8).cpu().detach())
 
log("Saving generated image grid")
cols = 4
rows = len(results) * (8 // cols)
fig, axes = plt.subplots(rows, cols, figsize=(cols * 3, rows * 3))
if rows == 1:
    axes = np.expand_dims(axes, axis=0)
 
for i, imgs in enumerate(results):
    for j, im in enumerate(imgs):
        r = i * (8 // cols) + j // cols
        c = j % cols
        if r < rows and c < cols:
            axes[r][c].imshow(im.squeeze(), cmap='gray')
            axes[r][c].axis('off')
 
plt.tight_layout()
plt.savefig(os.path.join(run_path, "generated.png"))
plt.close()
 
# ================== COMPARISON ==================
with torch.no_grad():
    real_imgs, _ = next(iter(dataloader))
    fake_imgs    = generator(torch.randn(8, N_QUBITS).to(device)).view(8, 1, 8, 8).cpu()
 
log("Saving real/fake comparison grid")
fig, axes = plt.subplots(2, 8, figsize=(16, 4))
for i in range(min(8, real_imgs.size(0), fake_imgs.size(0))):
    axes[0, i].imshow(real_imgs[i].squeeze(), cmap='gray')
    axes[0, i].axis('off')
    axes[0, i].set_title("Real")
    axes[1, i].imshow(fake_imgs[i].squeeze(), cmap='gray')
    axes[1, i].axis('off')
    axes[1, i].set_title("Fake")
plt.savefig(os.path.join(run_path, "comparison.png"))
plt.close()
 
# ================== METRICS ==================
log("Calculating metrics...")
 
def calculate_fid_simple(real, fake):
    real_flat = real.view(real.size(0), -1).numpy()
    fake_flat = fake.view(fake.size(0), -1).numpy()
    mu1, sigma1 = real_flat.mean(axis=0), np.cov(real_flat, rowvar=False)
    mu2, sigma2 = fake_flat.mean(axis=0), np.cov(fake_flat, rowvar=False)
    sigma1 += np.eye(sigma1.shape[0]) * 1e-6
    sigma2 += np.eye(sigma2.shape[0]) * 1e-6
    diff    = mu1 - mu2
    covmean = sqrtm(sigma1 @ sigma2)
    if np.iscomplexobj(covmean):
        covmean = covmean.real
    return float(diff @ diff + np.trace(sigma1 + sigma2 - 2 * covmean))
 
def calculate_ssim_simple(real, fake):
    real_np, fake_np = real.numpy(), fake.numpy()
    scores = []
    for i in range(len(real_np)):
        try:
            scores.append(ssim(real_np[i].squeeze(), fake_np[i].squeeze(), data_range=2.0))
        except Exception:
            scores.append(0.0)
    return float(np.mean(scores)) if scores else 0.0
 
def calculate_diversity_simple(fake):
    fake      = fake.view(fake.size(0), -1)
    n         = min(100, len(fake))
    distances = [torch.norm(fake[i] - fake[j]).item()
                 for i in range(n) for j in range(i + 1, n)]
    return float(np.mean(distances)) if distances else 0.0
 
def generate_fake_images(total_samples):
    chunks = []
    with torch.no_grad():
        for start in range(0, total_samples, BATCH_SIZE):
            nb    = min(BATCH_SIZE, total_samples - start)
            noise = torch.randn(nb, N_QUBITS).to(device)
            chunks.append(generator(noise).view(nb, 1, 8, 8).cpu().detach())
    return torch.cat(chunks, dim=0)
 
EVAL_SAMPLES = min(EVAL_SAMPLES_REQUESTED, len(dataset))
log(f"Evaluating with {EVAL_SAMPLES} samples...")
 
real_list = []
for data, _ in dataloader:
    real_list.append(data)
    if len(real_list) * data.size(0) >= EVAL_SAMPLES:
        break
 
real_batch = torch.cat(real_list, dim=0)[:EVAL_SAMPLES]
fake_batch = generate_fake_images(EVAL_SAMPLES)
 
fid_score  = calculate_fid_simple(real_batch, fake_batch)
ssim_score = calculate_ssim_simple(real_batch, fake_batch)
div_score  = calculate_diversity_simple(fake_batch)
 
log(f"FID: {fid_score:.4f}")
log(f"SSIM: {ssim_score:.4f}")
log(f"Diversity: {div_score:.4f}")
 
# ================== SAVE METRICS ==================
metrics = {
    "run_id":         run_id,
    "hardware_used":  USE_REAL_HARDWARE,
    "iterations":     counter,
    "n_qubits":       N_QUBITS,
    "batch_size":     BATCH_SIZE,
    "q_depth":        Q_DEPTH,
    "critic_steps":   CRITIC_STEPS,
    "lambda_gp":      LAMBDA_GP,
    "evaluation_metrics": {
        "FID":       fid_score,
        "SSIM":      ssim_score,
        "Diversity": div_score
    }
}
 
with open(os.path.join(run_path, "metrics.json"), "w") as f:
    json.dump(metrics, f, indent=4)
 
log(f"✅ Completed! Results saved in {run_path}")
log(f"Final metrics — FID: {fid_score:.4f}, SSIM: {ssim_score:.4f}, Diversity: {div_score:.4f}")