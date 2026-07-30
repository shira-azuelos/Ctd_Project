# KungFu Chess — Distributed Authoritative Real-Time System

![C++17](https://img.shields.io/badge/C%2B%2B-17-blue.svg)
![Docker](https://img.shields.io/badge/Docker-Compose-2496ED.svg)
![Kubernetes](https://img.shields.io/badge/Kubernetes-K3s-326CE5.svg)
![Redis](https://img.shields.io/badge/Redis-7.0-DC382D.svg)
![PostgreSQL](https://img.shields.io/badge/PostgreSQL-16-4169E1.svg)
![WebSocket](https://img.shields.io/badge/WebSocket-Async--IO-010101.svg)

High-performance, multi-computer real-time chess engine where both players execute moves simultaneously without turn constraints. The system features airborne piece trajectories, mid-air collisions, authoritative C++ engine validation, responsive rendering, Docker orchestration, and persistent PostgreSQL and Redis integration.

---

## Architecture Overview (6 Core System Components)

The architecture follows a distributed authoritative model designed to handle concurrent game instances at scale (target volume: 10 Million Concurrent Connected Users / 10M CCU).

```mermaid
graph TD
    Client1["Client PC 1 (Windows CLI / OpenCV)"]
    Client2["Client PC 2 (Windows CLI / OpenCV)"]
    
    subgraph Edge & Gateway Layer
        APIGW["1. API Gateway\n(Auth, Users, History)"]
        WSGW["2. WebSocket Gateway\n(Live Conn & 30 FPS State Stream)"]
    end
    
    subgraph Orchestration & Matchmaking
        MM["3. Matchmaker\n(ELO Queue +-100)"]
        GA["4. Game Allocator\n(Shards Assignment)"]
    end
    
    subgraph Core Engine Shards
        GS1["5. Game Server Shard 1\n(Authoritative C++ Engine)"]
        GS2["5. Game Server Shard 2\n(Authoritative C++ Engine)"]
    end
    
    subgraph Data & Persistence Layer
        Redis[("Redis 7\nSessions, Active Rooms, Reconnect")]
        Postgres[("PostgreSQL 16\nUsers, ELO, Game History")]
    end
    
    subgraph Observability
        Obs["6. Observability & Load Tester\n(Logs, C++ Native Load Testing)"]
    end

    Client1 -->|WS / Port 8080| WSGW
    Client2 -->|WS / Port 8080| WSGW
    Client1 -->|HTTP Login| APIGW
    Client2 -->|HTTP Login| APIGW
    
    APIGW --> Postgres
    WSGW <--> Redis
    WSGW --> MM
    MM --> GA
    GA --> GS1
    GA --> GS2
    GS1 <--> Redis
    GS2 <--> Redis
    GS1 --> Postgres
    Obs -.->|Monitors & Tests| GS1
```

---

## Component Breakdown

| Component | Functionality | Implementation / Reference |
|---|---|---|
| **1. API Gateway** | Authentication, user registration, and persistent record access | `src/io/user_manager.cpp` |
| **2. WebSocket Gateway** | Manages persistent client connections and 30 FPS state broadcasts | `src/network/socket_server.cpp` |
| **3. Matchmaker** | ELO-based matchmaking queue (+-100 ELO tolerance) | `SocketServer::process_matchmaking` |
| **4. Game Allocator** | Room assignment and server shard routing | `SocketServer::m_rooms` |
| **5. Game Server Shards** | Authoritative C++ GameEngine (Single Source of Truth) | `src/engine/game_engine.cpp` |
| **6. Observability** | Native C++ load testing, activity logging, and system metrics | `tests/load/load_tester.cpp` |

---

## Scale and Traffic Estimates (10M CCU Scale)

- **Concurrent Active Games**: 5,000,000 active room instances.
- **Upstream Network Ingestion**: ~0.8 Gbps aggregated move payload volume.
- **Downstream Broadcast Payload**: ~480 Gbps aggregated state update volume at 30 FPS.
- **Shard Container Topology**: Distributed across worker container instances (25,000 active rooms per shard).

---

## Key Technical Features

- **Authoritative C++ Engine**: Game state and rules are computed entirely on the server; client input acts solely as execution requests.
- **Zero-Latency Event Dispatching**: State updates are broadcast immediately upon validating player input.
- **Responsive Full-Screen Display**: Responsive OpenCV windowing configured for native monitor resolution without aspect distortion.
- **Multi-Computer LAN & Wi-Fi Operations**: Supports remote socket connections across network interfaces (0.0.0.0 binding).
- **Session Reconnection and Graceful Handling**: 20-second reconnection window for temporary network interruptions; automatic termination upon dual disconnect.
- **Persistence Layer Integration**: Real-time room caching backed by Redis in-memory data store alongside PostgreSQL relational storage.

---

## Getting Started

### Prerequisites

- Windows 10/11 (x64) or Linux environment
- MSVC C++17 Compiler / Visual Studio 2022
- OpenCV 4.x C++ SDK
- Docker and Docker Compose

---

## Execution and Deployment

### 1. Docker Compose Deployment

To compile and launch the full service stack (C++ Game Server, Redis, PostgreSQL):

```cmd
docker-compose up --build
```

### 2. Multi-Computer LAN Configuration

**Server Host (Computer 1):**
```cmd
compile_app.bat
app.exe client <SERVER_IP> 8080 <username_1> <password>
```

**Client Host (Computer 2):**
```cmd
app.exe client <SERVER_IP> 8080 <username_2> <password>
```

---

## Data Inspection Commands

### Query PostgreSQL User Records and ELO
```cmd
docker exec -it kungfu_postgres psql -U shira -d kungfu_chess -c "SELECT * FROM users;"
```

### Query PostgreSQL Game Logs
```cmd
docker exec -it kungfu_postgres psql -U shira -d kungfu_chess -c "SELECT * FROM games;"
```

### Inspect Active Redis Keys
```cmd
docker exec -it kungfu_redis redis-cli KEYS "*"
```

---

## Automated Load Testing

Run the native C++ load test simulating 50 concurrent client connections:

```cmd
run_load_test.bat
```

---

## Author

- **Shira Azuelos** — Computer Science Engineering Project
