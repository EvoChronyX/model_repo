<template>
  <section>
    <h2>Welcome</h2>
    <p>Manage your movies using Vue Router and Spring Boot.</p>

    <RouterLink to="/movies">
      <button>View All Movies</button>
    </RouterLink>
  </section>
</template>

<script setup>
import { RouterLink } from 'vue-router'
</script>