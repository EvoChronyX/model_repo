import mlflow
from sklearn.linear_model import LogisticRegression
from sklearn.datasets import load_iris
from sklearn.model_selection import train_test_split
from sklearn.metrics import accuracy_score

mlflow.set_tracking_uri("file:./mlruns")
mlflow.set_experiment("Food_Order_Prediction")

# Load dataset
data = load_iris()
X = data.data
y = data.target

# Split data
X_train, X_test, y_train, y_test = train_test_split(X, y, test_size=0.2)

# Start MLflow tracking
with mlflow.start_run(run_name="Logistics_run"):

    # Parameter
    model = LogisticRegression(max_iter=200)
    mlflow.log_param("model_type", "LogisticRegression")
    mlflow.log_param("max_iter", 200)

    # Train model
    model.fit(X_train, y_train)

    # Predict
    y_pred = model.predict(X_test)

    # Metric
    acc = accuracy_score(y_test, y_pred)
    mlflow.log_metric("accuracy", acc)

    print("Accuracy:", acc)