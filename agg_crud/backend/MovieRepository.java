package com.example.agg_crud_backend.repository;


import com.example.agg_crud_backend.model.Movie;
import org.springframework.data.mongodb.repository.MongoRepository;
import org.springframework.data.mongodb.repository.Aggregation;
import java.util.List;
import java.util.Map;

public interface MovieRepository
        extends MongoRepository<Movie, String> {

    // AGGREGATION: Fetch only movies having bookings
    @Aggregation(pipeline = {
            "{ '$lookup': { 'from': 'booking', 'localField': 'movieId', 'foreignField': 'movieId', 'as': 'bookings' } }",
            "{ '$match': { 'bookings.0': { '$exists': true } } }",
            "{ '$project': { '_id': 0, 'movieName': 1, 'genre': 1 } }"
    })
    List<Map<String, Object>> findBookedMovies();

    // CRUD is already provided by MongoRepository:
    // save(movie)       -> Create or update
    // findAll()         -> Read all
    // findById(id)      -> Read one
    // deleteById(id)    -> Delete
}


//spring.application.name=agg_crud_backend
//spring.data.mongodb.uri=mongodb://localhost:27017/movieDB
//server.port=8080