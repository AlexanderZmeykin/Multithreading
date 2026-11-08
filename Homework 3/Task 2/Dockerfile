# syntax=docker/dockerfile:1.7

# Сборка для GCC

FROM gcc:13 AS build-gcc

WORKDIR /src
COPY main.cpp .

RUN mkdir -p /out && \
    g++ -O2 -std=c++17 -Wall -Wextra \
        -static-libstdc++ -static-libgcc \
        -o /out/app-gcc main.cpp

# Она же для clang-а

FROM debian:bookworm AS build-clang

RUN apt-get update \
 && apt-get install -y --no-install-recommends clang \
 && rm -rf /var/lib/apt/lists/*

WORKDIR /src
COPY main.cpp .

RUN mkdir -p /out && \
    clang++ -O2 -std=c++17 -Wall -Wextra \
        -static-libstdc++ -static-libgcc \
        -o /out/app-clang main.cpp


FROM debian:bookworm-slim

# libstdc++6 нужен на случай динамической линковки;
# при -static-libstdc++ формально не обязателен, но безвреден.
RUN apt-get update \
 && apt-get install -y --no-install-recommends libstdc++6 \
 && rm -rf /var/lib/apt/lists/*

COPY --from=build-gcc   /out/app-gcc   /usr/local/bin/app-gcc
COPY --from=build-clang /out/app-clang /usr/local/bin/app-clang

CMD ["/bin/bash"]