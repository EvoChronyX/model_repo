package com.example.agg_crud_backend.repository;

import com.example.agg_crud_backend.model.Movie;
import org.springframework.data.mongodb.repository.MongoRepository;

public interface MovieRepository
        extends MongoRepository<Movie, String> {
}