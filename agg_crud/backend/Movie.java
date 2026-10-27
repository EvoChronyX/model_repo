package com.example.agg_crud_backend.model;


import lombok.Data;
import org.springframework.data.annotation.Id;
import org.springframework.data.mongodb.core.mapping.Document;

@Data
@Document(collection = "movies")
public class Movie {

    @Id
    private String id;

    private String movieId;
    private String movieName;
    private String genre;
    private String director;
    private int duration;
}