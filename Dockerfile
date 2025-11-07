# Use Ubuntu base image
FROM ubuntu:24.04

# Install dependencies
RUN apt update && apt install -y g++ make libpq-dev curl git

# Set working directory
WORKDIR /app

# Copy project files into container
COPY . /app

# Compile your C++ server
RUN g++ server.cpp database.cpp cache.cpp -o server -lpq -pthread -std=c++17 -I/usr/include/postgresql

# Expose port 8080 for HTTP
EXPOSE 8080

# Run the server when container starts
CMD ["./server"]
