FROM ubuntu:22.04 AS builder

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y \
    g++ \
    make \
    libopencv-dev \
    libboost-system-dev \
    libboost-thread-dev \
    libwebsocketpp-dev \
    nlohmann-json3-dev \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

COPY . .

RUN g++ -std=c++17 -O2 -Iinclude -I/usr/include/opencv4 \
    main.cpp \
    src/model/*.cpp \
    src/rules/*.cpp \
    src/realtime/*.cpp \
    src/engine/*.cpp \
    src/pubsub/*.cpp \
    src/io/*.cpp \
    src/network/*.cpp \
    src/view/*.cpp \
    src/input/*.cpp \
    -lopencv_core -lopencv_imgproc -lopencv_highgui -lopencv_imgcodecs -lboost_system -lboost_thread -lpthread \
    -o /app/app

FROM ubuntu:22.04 AS runner

ENV DEBIAN_FRONTEND=noninteractive
RUN apt-get update && apt-get install -y \
    libopencv-core4.5d \
    libopencv-imgproc4.5d \
    libopencv-highgui4.5d \
    libopencv-imgcodecs4.5d \
    libboost-system1.74.0 \
    libboost-thread1.74.0 \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

COPY --from=builder /app/app /app/app
COPY users.json /app/users.json

EXPOSE 8080
CMD ["/app/app", "server", "8080"]
