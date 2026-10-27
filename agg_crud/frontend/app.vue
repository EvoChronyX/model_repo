
<script setup>
import { ref, onMounted } from 'vue'
import axios from 'axios'

const movies = ref([])
const bookedMovies = ref([])
const movie = ref({
  movieId: '',
  movieName: '',
  genre: '',
  director: '',
  duration: 0
})

// Fetch all movies
async function getMovies() {
  const res = await axios.get('http://localhost:8080/api/movies')
  movies.value = res.data
}

// Aggregation: fetch only movies with bookings
async function getBookedMovies() {
  const res = await axios.get('http://localhost:8080/api/movies/booked')
  bookedMovies.value = res.data
}

// CREATE: POST /api/movies
// async function addMovie() {
//   await axios.post('http://localhost:8080/api/movies', movie.value)
//   await getMovies()
//   movie.value = { movieId: '', movieName: '', genre: '', director: '', duration: 0 }
// }

// UPDATE: PUT /api/movies/{id}
// async function updateMovie(m) {
//   await axios.put(`http://localhost:8080/api/movies/${m.id}`, m)
//   await getMovies()
// }

// DELETE: DELETE /api/movies/{id}
// async function deleteMovie(id) {
//   await axios.delete(`http://localhost:8080/api/movies/${id}`)
//   await getMovies()
// }

// READ ONE: GET /api/movies/{id}
// async function getMovie(id) {
//   const res = await axios.get(`http://localhost:8080/api/movies/${id}`)
//   console.log(res.data)
// }

onMounted(() => {
  getMovies()
  getBookedMovies()
})
</script>

<template>
  <main class="container">
    <h1>Movie Booking System</h1>

    <!-- CREATE FORM: uncomment addMovie and @submit.prevent="addMovie"
    <form @submit.prevent="addMovie" class="form">
      <input v-model="movie.movieId" placeholder="Movie ID" required />
      <input v-model="movie.movieName" placeholder="Movie Name" required />
      <input v-model="movie.genre" placeholder="Genre" required />
      <input v-model="movie.director" placeholder="Director" />
      <input v-model.number="movie.duration" type="number" placeholder="Duration" />
      <button type="submit">Add Movie</button>
    </form>
    -->

    <section>
      <h2>All Movies</h2>

      <table>
        <thead>
          <tr>
            <th>Movie ID</th>
            <th>Name</th>
            <th>Genre</th>
            <th>Director</th>
            <th>Duration</th>
            <!-- <th>Actions</th> -->
          </tr>
        </thead>

        <tbody>
          <tr v-for="m in movies" :key="m.id">
            <td>{{ m.movieId }}</td>
            <td>{{ m.movieName }}</td>
            <td>{{ m.genre }}</td>
            <td>{{ m.director }}</td>
            <td>{{ m.duration }}</td>

            <!-- CRUD buttons: uncomment after enabling functions
            <td>
              <button @click="updateMovie(m)">Update</button>
              <button @click="deleteMovie(m.id)">Delete</button>
              <button @click="getMovie(m.id)">View</button>
            </td>
            -->
          </tr>
        </tbody>
      </table>
    </section>

    <section>
      <h2>Movies with Bookings (Aggregation)</h2>

      <table>
        <thead>
          <tr>
            <th>Movie Name</th>
            <th>Genre</th>
          </tr>
        </thead>

        <tbody>
          <tr v-for="m in bookedMovies" :key="m.movieName">
            <td>{{ m.movieName }}</td>
            <td>{{ m.genre }}</td>
          </tr>
        </tbody>
      </table>

      <button @click="getBookedMovies">Refresh Booked Movies</button>
    </section>
  </main>
</template>

<style>
/* Main page: max-width limits content width; margin auto centers it */
.container {
  max-width: 900px;
  margin: 30px auto;
  padding: 20px;
  font-family: Arial, sans-serif;
  color: #222;
}

/* Heading: color changes the text color; text-align controls alignment */
h1 {
  text-align: center;
  color: #234;
}

/* Section spacing */
section {
  margin-top: 30px;
}

/* Table: width, border-collapse and margin control layout */
table {
  width: 100%;
  border-collapse: collapse;
  margin: 15px 0;
}

/* Cell styling: padding controls space inside each cell */
th, td {
  border: 1px solid #ccc;
  padding: 10px;
  text-align: left;
}

/* Header background and text color */
th {
  background: #eee;
}

/* Hover effect: changes a row's background when the mouse moves over it */
tbody tr:hover {
  background: #f5f5f5;
}

/* Button styling: background, color, padding, border-radius */
button {
  padding: 8px 14px;
  margin: 5px;
  background: #234;
  color: white;
  border: none;
  border-radius: 4px;
  cursor: pointer;
}

/* Hover state for buttons */
button:hover {
  background: #456;
}

/* Inputs: width, padding and border */
input {
  padding: 8px;
  margin: 5px;
  border: 1px solid #ccc;
  border-radius: 4px;
}

/* Responsive option: allows horizontal scrolling on narrow screens */
/* Wrap a table in <div class="table-wrap"> and use overflow-x: auto */
</style>
