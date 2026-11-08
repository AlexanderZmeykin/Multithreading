FROM gcc:latest

WORKDIR /app
COPY main.cpp .
RUN g++ -o main main.cpp

CMD ["./main"]