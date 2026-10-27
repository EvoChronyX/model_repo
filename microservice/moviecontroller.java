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

    @GetMapping
    public List<Movie> getMovies() {
        return repo.findAll();
    }

    @PostMapping
    public Movie addMovie(@RequestBody Movie movie) {
        return repo.save(movie);
    }
}

//This service runs on port 8081 and provides:
//  GET http://localhost:8081/api/movies
// POST http://localhost:8081/api/movies