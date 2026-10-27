<script setup>
import { ref, onMounted } from 'vue'
import axios from 'axios'
import { useRoute, useRouter } from 'vue-router'

const route = useRoute()
const router = useRouter()

const movie = ref({
  movieId: '',
  movieName: '',
  genre: '',
  director: '',
  duration: 0
})

const id = route.params.id

onMounted(async () => {
  if (id) {
    const res = await axios.get(
      `http://localhost:8080/api/movies/${id}`
    )
    movie.value = res.data
  }
})

async function saveMovie() {
  if (id) {
    await axios.put(
      `http://localhost:8080/api/movies/${id}`,
      movie.value
    )
  } else {
    await axios.post(
      'http://localhost:8080/api/movies',
      movie.value
    )
  }

  router.push('/movies')
}
</script>

<template>
  <section>
    <h2>{{ id ? 'Edit Movie' : 'Add Movie' }}</h2>

    <form @submit.prevent="saveMovie">
      <p>
        <input v-model="movie.movieId" placeholder="Movie ID" required />
      </p>

      <p>
        <input v-model="movie.movieName" placeholder="Movie Name" required />
      </p>

      <p>
        <input v-model="movie.genre" placeholder="Genre" required />
      </p>

      <p>
        <input v-model="movie.director" placeholder="Director" />
      </p>

      <p>
        <input
          v-model.number="movie.duration"
          type="number"
          placeholder="Duration"
        />
      </p>

      <button type="submit">
        {{ id ? 'Update Movie' : 'Save Movie' }}
      </button>

      <button type="button" @click="router.push('/movies')">
        Cancel
      </button>
    </form>
  </section>
</template>