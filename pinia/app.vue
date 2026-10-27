
<script setup>
import { ref, onMounted } from 'vue'
import { storeToRefs } from 'pinia'
import { useMovieStore } from './stores/movieStore'

const store = useMovieStore()
const { movies } = storeToRefs(store)

const form = ref({
  movieId: '',
  movieName: '',
  genre: '',
  director: '',
  duration: 0
})

const editingId = ref(null)

onMounted(() => {
  store.fetchMovies()
})

async function submitMovie() {
  if (editingId.value) {
    await store.updateMovie(editingId.value, form.value)
  } else {
    await store.addMovie(form.value)
  }

  resetForm()
}

function editMovie(movie) {
  editingId.value = movie.id
  form.value = { ...movie }
}

function resetForm() {
  editingId.value = null
  form.value = {
    movieId: '',
    movieName: '',
    genre: '',
    director: '',
    duration: 0
  }
}

async function removeMovie(id) {
  if (confirm('Delete this movie?')) {
    await store.deleteMovie(id)

    if (editingId.value === id) {
      resetForm()
    }
  }
}
</script>

<template>
  <main class="container">
    <h1>Movie Management</h1>

    <form @submit.prevent="submitMovie" class="form">
      <h2>{{ editingId ? 'Update Movie' : 'Add Movie' }}</h2>

      <input v-model="form.movieId" placeholder="Movie ID" required />
      <input v-model="form.movieName" placeholder="Movie Name" required />
      <input v-model="form.genre" placeholder="Genre" required />
      <input v-model="form.director" placeholder="Director" />

      <input
        v-model.number="form.duration"
        type="number"
        placeholder="Duration"
        min="0"
      />

      <button type="submit">
        {{ editingId ? 'Update Movie' : 'Add Movie' }}
      </button>

      <button
        v-if="editingId"
        type="button"
        class="cancel"
        @click="resetForm"
      >
        Cancel
      </button>
    </form>

    <h2>All Movies</h2>

    <table>
      <thead>
        <tr>
          <th>Movie ID</th>
          <th>Name</th>
          <th>Genre</th>
          <th>Director</th>
          <th>Duration</th>
          <th>Actions</th>
        </tr>
      </thead>

      <tbody>
        <tr v-for="m in movies" :key="m.id">
          <td>{{ m.movieId }}</td>
          <td>{{ m.movieName }}</td>
          <td>{{ m.genre }}</td>
          <td>{{ m.director }}</td>
          <td>{{ m.duration }}</td>
          <td>
            <button @click="editMovie(m)">Edit</button>
            <button class="delete" @click="removeMovie(m.id)">
              Delete
            </button>
          </td>
        </tr>

        <tr v-if="movies.length === 0">
          <td colspan="6">No movies found.</td>
        </tr>
      </tbody>
    </table>
  </main>
</template>

<style>
.container {
  max-width: 1000px;
  margin: 30px auto;
  padding: 20px;
  font-family: Arial, sans-serif;
  color: #222;
}

h1 {
  text-align: center;
}

.form {
  display: flex;
  flex-wrap: wrap;
  gap: 10px;
  margin-bottom: 30px;
  padding: 20px;
  background: #f5f5f5;
  border-radius: 8px;
}

.form h2 {
  width: 100%;
}

input {
  flex: 1 1 180px;
  min-width: 0;
  padding: 10px;
  border: 1px solid #ccc;
  border-radius: 4px;
}

button {
  padding: 9px 14px;
  margin: 2px;
  border: none;
  border-radius: 4px;
  background: #234;
  color: white;
  cursor: pointer;
}

button:hover {
  background: #456;
}

.delete {
  background: #b42318;
}

.cancel {
  background: #666;
}

table {
  width: 100%;
  border-collapse: collapse;
}

th, td {
  padding: 10px;
  border: 1px solid #ccc;
  text-align: left;
}

th {
  background: #eee;
}

tbody tr:hover {
  background: #f5f5f5;
}
</style>
