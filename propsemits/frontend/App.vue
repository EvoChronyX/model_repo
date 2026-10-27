
<script setup>
import { ref, onMounted } from 'vue'
import axios from 'axios'
import MovieForm from './components/MovieForm.vue'

const API = 'http://localhost:8080/api/movies'

const movies = ref([])
const selectedMovie = ref(null)
const showForm = ref(false)

async function getMovies() {
  const res = await axios.get(API)
  movies.value = res.data
}

function addMovie() {
  selectedMovie.value = null
  showForm.value = true
}

function editMovie(movie) {
  selectedMovie.value = { ...movie }
  showForm.value = true
}

async function saveMovie(movie) {
  if (selectedMovie.value?.id) {
    await axios.put(API + '/' + selectedMovie.value.id, movie)
  } else {
    await axios.post(API, movie)
  }

  showForm.value = false
  selectedMovie.value = null
  await getMovies()
}

async function deleteMovie(id) {
  await axios.delete(API + '/' + id)
  await getMovies()
}

function cancelForm() {
  showForm.value = false
  selectedMovie.value = null
}

onMounted(getMovies)
</script>

<template>
  <div class="container">
    <h1>Movie Management</h1>

    <button @click="addMovie">Add Movie</button>

    <MovieForm
      v-if="showForm"
      :movie="selectedMovie"
      @save="saveMovie"
      @cancel="cancelForm"
    />

    <h2>Movie List</h2>

    <div class="table-box">
      <table>
        <thead>
          <tr>
            <th>ID</th>
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
            <td>{{ m.duration }} min</td>
            <td>
              <button @click="editMovie(m)">Edit</button>
              <button class="delete"
                      @click="deleteMovie(m.id)">Delete</button>
            </td>
          </tr>

          <tr v-if="movies.length === 0">
            <td colspan="6">No movies found.</td>
          </tr>
        </tbody>
      </table>
    </div>
  </div>
</template>

<style>
* {
  box-sizing: border-box;
}

body {
  margin: 0;
  font-family: Arial, sans-serif;
  background: #f1f5f9;
  color: #1e293b;
}

.container {
  max-width: 1000px;
  margin: 35px auto;
  padding: 20px;
}

h1, h2 {
  color: #1e3a5f;
}

button {
  padding: 9px 14px;
  margin: 4px;
  border: none;
  border-radius: 5px;
  background: #2563eb;
  color: white;
  cursor: pointer;
}

button:hover {
  opacity: 0.85;
}

.delete {
  background: #dc2626;
}

.cancel {
  background: #64748b;
}

.card {
  background: white;
  padding: 20px;
  margin: 20px 0;
  border-radius: 10px;
  box-shadow: 0 2px 8px #cbd5e1;
}

input {
  display: block;
  width: 100%;
  padding: 11px;
  margin: 10px 0;
  border: 1px solid #cbd5e1;
  border-radius: 5px;
}

.table-box {
  overflow-x: auto;
}

table {
  width: 100%;
  margin-top: 15px;
  border-collapse: collapse;
  background: white;
}

th, td {
  padding: 12px;
  text-align: left;
  border-bottom: 1px solid #e2e8f0;
}

th {
  background: #1e3a5f;
  color: white;
}

tr:hover {
  background: #f8fafc;
}
</style>
