killall -9 main
go run $GOPATH/src/github.com/ucbrise/MerkleSquare/demo/server/main.go > server.log 2>&1 &
go run $GOPATH/src/github.com/ucbrise/MerkleSquare/demo/verifier/main.go > verifier.log 2>&1 &
go run $GOPATH/src/github.com/ucbrise/MerkleSquare/demo/auditor/main.go > auditor.log 2>&1 &
