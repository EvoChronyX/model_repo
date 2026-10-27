package com.example.agg_crud_backend.model;



import lombok.Data;
import org.springframework.data.annotation.Id;
import org.springframework.data.mongodb.core.mapping.Document;

@Data
@Document(collection = "booking")
public class Booking {
    @Id
    private String id;

    private String movieId;
    private int ticketsBooked;
    private double totalPrice;
}