<script setup>
import { ref, onMounted } from 'vue'
import axios from 'axios'
import { useRouter } from 'vue-router'

const movies = ref([])
const router = useRouter()

async function getMovies() {
  const res = await axios.get('http://localhost:8080/api/movies')
  movies.value = res.data
}

async function deleteMovie(id) {
  if (confirm('Delete this movie?')) {
    await axios.delete(`http://localhost:8080/api/movies/${id}`)
    await getMovies()
  }
}

function editMovie(id) {
  router.push(`/edit/${id}`)
}

onMounted(getMovies)
</script>

<template>
  <section>
    <h2>All Movies</h2>

    <button @click="router.push('/add')">Add Movie</button>

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
            <button @click="editMovie(m.id)">Edit</button>
            <button @click="deleteMovie(m.id)">Delete</button>
          </td>
        </tr>

        <tr v-if="movies.length === 0">
          <td colspan="6">No movies found</td>
        </tr>
      </tbody>
    </table>
  </section>
</template>