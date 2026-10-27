import { createRouter, createWebHistory } from 'vue-router'

import HomeView from '../views/HomeView.vue'
import MovieList from '../views/MovieList.vue'
import MovieForm from '../views/MovieForm.vue'

const routes = [
  { path: '/', component: HomeView },
  { path: '/movies', component: MovieList },
  { path: '/add', component: MovieForm },
  { path: '/edit/:id', component: MovieForm }
]

const router = createRouter({
  history: createWebHistory(),
  routes
})

export default router