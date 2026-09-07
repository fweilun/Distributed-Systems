1. 
performance server on server #1 (send after the whole process)
metrics:
    machine_type: server | client, machine_id, metrics_name, time

stored in metrics.log



2. 
A log connected to server.cpp, client.cpp
utilize the same logic to grep the logs
file_name: machine.i.metrics.log
metrics:
    machine_type: server | client, machine_id, metrics_name, time

struct Metrics {
    machine_type: server | client, 
    machine_id: int, 1..n
    metrics_name: string,
    time: time?
}

string path;

static func Metrics::set_path(string& path) {
    
}

static func Metrics::write(metrics) {

}

static func Metrics::load() {

}

init()
record(metrics)

