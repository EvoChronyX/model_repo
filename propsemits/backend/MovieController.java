package com.example.agg_crud_backend.controller;

import com.example.agg_crud_backend.model.Movie;
import com.example.agg_crud_backend.repository.MovieRepository;
import org.springframework.web.bind.annotation.*;
import java.util.List;

@RestController
@RequestMapping("/api/movies")
@CrossOrigin(origins = "http://localhost:5173")
public class MovieController {

    private final MovieRepository repo;

    public MovieController(MovieRepository repo) {
        this.repo = repo;
    }

    @PostMapping
    public Movie add(@RequestBody Movie movie) {
        return repo.save(movie);
    }

    @GetMapping
    public List<Movie> getAll() {
        return repo.findAll();
    }

    @PutMapping("/{id}")
    public Movie update(@PathVariable String id,
                        @RequestBody Movie movie) {
        movie.setId(id);
        return repo.save(movie);
    }

    @DeleteMapping("/{id}")
    public void delete(@PathVariable String id) {
        repo.deleteById(id);
    }
}