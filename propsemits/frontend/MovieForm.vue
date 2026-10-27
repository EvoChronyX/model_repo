
<script setup>
import { reactive, watch } from 'vue'
const props = defineProps(['movie'])
const emit = defineEmits(['save', 'cancel'])

const form = reactive({
  movieId: '',
  movieName: '',
  genre: '',
  director: '',
  duration: 0
})

watch(() => props.movie, (movie) => {
  Object.assign(form, movie || {
    movieId: '',
    movieName: '',
    genre: '',
    director: '',
    duration: 0
  })
}, { immediate: true })

function submitForm() {
  emit('save', { ...form })
}
</script>

<template>
  <div class="card">
    <h2>{{ movie ? 'Edit Movie' : 'Add Movie' }}</h2>

    <form @submit.prevent="submitForm">
      <input v-model="form.movieId" placeholder="Movie ID" required />
      <input v-model="form.movieName" placeholder="Movie Name" required />
      <input v-model="form.genre" placeholder="Genre" required />
      <input v-model="form.director" placeholder="Director" required />
      <input v-model.number="form.duration" type="number"
             placeholder="Duration (minutes)" min="1" required />

      <button type="submit">Save</button>
      <button type="button" class="cancel"
              @click="emit('cancel')">Cancel</button>
    </form>
  </div>
</template>
