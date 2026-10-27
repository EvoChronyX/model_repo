
<script setup>
import { ref, onMounted } from 'vue'
import axios from 'axios'

const movieAPI = 'http://localhost:8081/api/movies'
const reviewAPI = 'http://localhost:8082/api/reviews'

const movies = ref([])
const reviews = ref([])

const movie = ref({
  movieId: '',
  movieName: '',
  genre: '',
  director: '',
  duration: 120
})

const review = ref({
  movieName: '',
  comment: ''
})

async function getMovies() {
  const res = await axios.get(movieAPI)
  movies.value = res.data
}

async function getReviews() {
  const res = await axios.get(reviewAPI)
  reviews.value = res.data
}

async function addMovie() {
  await axios.post(movieAPI, movie.value)

  movie.value = {
    movieId: '',
    movieName: '',
    genre: '',
    director: '',
    duration: 120
  }

  await getMovies()
}

async function addReview() {
  await axios.post(reviewAPI, review.value)

  review.value = { movieName: '', comment: '' }
  await getReviews()
}

onMounted(() => {
  getMovies()
  getReviews()
})
</script>

<template>
  <main class="container">
    <h1>Microservices Demo</h1>
    <p>Movie Service: 8081 | Review Service: 8082</p>

    <section class="card">
      <h2>Add Movie</h2>

      <form @submit.prevent="addMovie">
        <input v-model="movie.movieId" placeholder="Movie ID" required />
        <input v-model="movie.movieName" placeholder="Movie Name" required />
        <input v-model="movie.genre" placeholder="Genre" required />
        <input v-model="movie.director" placeholder="Director" required />
        <input v-model.number="movie.duration" type="number"
               placeholder="Duration" min="1" required />
        <button type="submit">Save Movie</button>
      </form>

      <h3>Movies</h3>
      <ul>
        <li v-for="m in movies" :key="m.id">
          {{ m.movieName }} — {{ m.genre }}
        </li>
      </ul>
    </section>

    <section class="card">
      <h2>Add Review</h2>

      <form @submit.prevent="addReview">
        <input v-model="review.movieName"
               placeholder="Movie Name" required />
        <textarea v-model="review.comment"
                  placeholder="Write a review" required />
        <button type="submit">Save Review</button>
      </form>

      <h3>Reviews</h3>
      <ul>
        <li v-for="r in reviews" :key="r.id">
          <strong>{{ r.movieName }}:</strong> {{ r.comment }}
        </li>
      </ul>
    </section>
  </main>
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
  max-width: 850px;
  margin: 30px auto;
  padding: 20px;
}

h1 {
  color: #1e3a5f;
}

.card {
  background: white;
  padding: 20px;
  margin: 20px 0;
  border-radius: 10px;
  box-shadow: 0 2px 8px #cbd5e1;
}

input, textarea {
  display: block;
  width: 100%;
  padding: 10px;
  margin: 10px 0;
  border: 1px solid #cbd5e1;
  border-radius: 5px;
  font: inherit;
}

textarea {
  min-height: 80px;
}

button {
  padding: 10px 16px;
  border: none;
  border-radius: 5px;
  background: #2563eb;
  color: white;
  cursor: pointer;
}

li {
  margin: 10px 0;
  overflow-wrap: anywhere;
}
</style>
