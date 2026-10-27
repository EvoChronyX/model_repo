<script setup>
import { RouterLink, RouterView } from 'vue-router'
</script>

<template>
  <div class="container">
    <h1>Movie Management System</h1>

    <nav>
      <RouterLink to="/">Home</RouterLink>
      <RouterLink to="/movies">View Movies</RouterLink>
      <RouterLink to="/add">Add Movie</RouterLink>
    </nav>

    <RouterView />
  </div>
</template>

<style>
body {
  margin: 0;
  background: #f4f6f9;
  font-family: Arial, sans-serif;
  color: #222;
}

.container {
  max-width: 1000px;
  margin: 30px auto;
  padding: 20px;
}

h1 {
  text-align: center;
  color: #234;
}

nav {
  display: flex;
  justify-content: center;
  flex-wrap: wrap;
  gap: 12px;
  margin: 20px 0 30px;
}

nav a {
  padding: 10px 16px;
  background: #234;
  color: white;
  text-decoration: none;
  border-radius: 5px;
}

nav a:hover {
  background: #456;
}

nav a.router-link-active {
  background: #167d69;
}

input {
  padding: 10px;
  border: 1px solid #ccc;
  border-radius: 5px;
}

button {
  padding: 9px 13px;
  margin: 4px;
  border: none;
  border-radius: 5px;
  background: #234;
  color: white;
  cursor: pointer;
}

button:hover {
  opacity: 0.85;
}

table {
  width: 100%;
  border-collapse: collapse;
  background: white;
  margin: 15px 0;
}

th, td {
  border: 1px solid #ddd;
  padding: 10px;
  text-align: left;
}

th {
  background: #e8edf3;
}

@media (max-width: 650px) {
  .container {
    margin: 10px;
    padding: 10px;
    overflow-x: auto;
  }

  table {
    min-width: 650px;
  }
}
</style>