echo { \"actions\": [ { \"name\": \"transition_service_to_open\", \"args\": { \"next_service_identity\": \"$(awk '{printf "%s\\n", $0}' service_cert.pem)\"} } ] } > transition_service_to_open.json
