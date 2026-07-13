import redis
import time
import threading

HOST = "localhost"
PORT = 6380
NUM_CLIENTS = 20
OPS_PER_CLIENT = 500

latencies = []
lock = threading.Lock()

def worker(client_id):
    r = redis.Redis(host=HOST, port=PORT, decode_responses=True, protocol=2)
    local_latencies = []
    for i in range(OPS_PER_CLIENT):
        key = f"client{client_id}:key{i}"
        value = f"value{i}"

        start = time.perf_counter()
        r.execute_command("SET", key, value)
        r.execute_command("GET", key)
        end = time.perf_counter()

        local_latencies.append(end - start)

    with lock:
        latencies.extend(local_latencies)

def main():
    threads = []
    start_time = time.perf_counter()

    for i in range(NUM_CLIENTS):
        t = threading.Thread(target=worker, args=(i,))
        threads.append(t)
        t.start()

    for t in threads:
        t.join()

    total_time = time.perf_counter() - start_time
    total_ops = NUM_CLIENTS * OPS_PER_CLIENT * 2  # SET + GET each counted

    latencies.sort()
    p50 = latencies[len(latencies) // 2] * 1000
    p99 = latencies[int(len(latencies) * 0.99)] * 1000

    print(f"\n--- Load Test Results ---")
    print(f"Clients: {NUM_CLIENTS}, Ops per client: {OPS_PER_CLIENT} (SET+GET pairs)")
    print(f"Total operations: {total_ops}")
    print(f"Total time: {total_time:.2f}s")
    print(f"Throughput: {total_ops / total_time:.0f} ops/sec")
    print(f"p50 latency: {p50:.2f} ms")
    print(f"p99 latency: {p99:.2f} ms")

if __name__ == "__main__":
    main()