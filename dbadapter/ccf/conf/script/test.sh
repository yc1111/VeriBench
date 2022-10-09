host=10.10.10.230:8080

curl https://${host}/app/log?id=1 -X POST \
    --cacert ../external/service_cert.pem \
    --key user0_privk.pem \
    --cert user0_cert.pem \
    --header 'Content-Type: application/json' \
    --data-binary @request.json

curl https://${host}/app/log?id=1 -X GET \
    --cacert ../external/service_cert.pem \
    --key user0_privk.pem \
    --cert user0_cert.pem
