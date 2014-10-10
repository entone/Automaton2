database_name = "automaton"

spaces= [ 
{
    "name": "events",
    "retentionPolicy": "7d",
    "shardDuration": "2h",
    "regex": "/.*/",
    "replicationFactor": 1,
    "split": 1
},
{
    "name": "5m_rollup",
    "retentionPolicy": "30d",
    "shardDuration": "1h",
    "regex": "/^5m.*/",
    "replicationFactor": 1,
    "split": 1
},
{
    "name": "30m_rollup",
    "retentionPolicy": "60d",
    "shardDuration": "1h",
    "regex": "/^30m.*/",
    "replicationFactor": 1,
    "split": 1
},
{
    "name": "1h_rollup",
    "retentionPolicy": "inf",
    "shardDuration": "5h",
    "regex": "/^1h.*/",
    "replicationFactor": 1,
    "split": 1
},
{
    "name": "24h_rollup",
    "retentionPolicy": "inf",
    "shardDuration": "24h",
    "regex": "/^24h.*/",
    "replicationFactor": 1,
    "split": 1
}]

queries = [
    "SELECT mean(value) as value FROM /^events.*/ GROUP BY time(5m) INTO 5m.:series_name.mean",
    "SELECT mean(value) as value FROM /^events.*/ GROUP BY time(30m) INTO 30m.:series_name.mean",
    "SELECT mean(value) as value FROM /^events.*/ GROUP BY time(1h) INTO 1h.:series_name.mean",
    "SELECT mean(value) as value FROM /^events.*/ GROUP BY time(24h) INTO 24h.:series_name.mean",
]