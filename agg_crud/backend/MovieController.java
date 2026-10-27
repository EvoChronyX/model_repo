package com.example.agg_crud_backend.controller;



import com.example.agg_crud_backend.model.Booking;
import com.example.agg_crud_backend.model.Movie;
import com.example.agg_crud_backend.repository.MovieRepository;

import org.springframework.data.mongodb.core.MongoTemplate;
import org.springframework.web.bind.annotation.*;
import org.springframework.http.ResponseEntity;
import java.util.List;
import java.util.Map;

@RestController
@RequestMapping("/api/movies")
@CrossOrigin(origins = "http://localhost:5173")
public class MovieController {

    private final MovieRepository repository;
    private final MongoTemplate mongoTemplate;

    public MovieController(MovieRepository repository,MongoTemplate mongoTemplate) {
        this.repository = repository;
        this.mongoTemplate = mongoTemplate;
    }

    // AGGREGATION: GET movies having bookings
    @GetMapping("/booked")
    public List<Map<String, Object>> getBookedMovies() {
        return repository.findBookedMovies();
    }


    // CREATE: POST /api/movies
    @PostMapping
    public Movie addMovie(@RequestBody Movie movie) {
        return repository.save(movie);
    }
    // READ ALL: GET /api/movies
    @GetMapping
    public List<Movie> getAllMovies() {
        return repository.findAll();
    }

    @PostMapping("/booking")
    public Booking addBooking(@RequestBody Booking booking) {
        return mongoTemplate.insert(booking, "booking");
    }
    /*
    // READ ONE: GET /api/movies/{id}
    @GetMapping("/{id}")
    public ResponseEntity<Movie> getMovie(@PathVariable String id) {
        return repository.findById(id)
            .map(ResponseEntity::ok)
            .orElse(ResponseEntity.notFound().build());
    }

    // UPDATE: PUT /api/movies/{id}
    @PutMapping("/{id}")
    public ResponseEntity<Movie> updateMovie(
            @PathVariable String id,
            @RequestBody Movie movie) {

        if (!repository.existsById(id)) {
            return ResponseEntity.notFound().build();
        }

        movie.setId(id);
        return ResponseEntity.ok(repository.save(movie));
    }

    // DELETE: DELETE /api/movies/{id}
    @DeleteMapping("/{id}")
    public ResponseEntity<Void> deleteMovie(@PathVariable String id) {
        if (!repository.existsById(id)) {
            return ResponseEntity.notFound().build();
        }

        repository.deleteById(id);
        return ResponseEntity.noContent().build();
    }
    */
}