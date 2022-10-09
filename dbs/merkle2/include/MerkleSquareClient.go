package main

import (
	"context"
	"errors"
	"github.com/immesys/bw2/crypto"
	"github.com/ucbrise/MerkleSquare/client"
	"sync"
)

type MerkleSquareClient struct {
	c   *client.Client
	ctx context.Context
}

func NewMerkleSquareClient(serverAddr, auditorAddr, verifierAddr string) *MerkleSquareClient {
	ctx := context.Background()
	c, err := client.NewClient(serverAddr, auditorAddr, verifierAddr)
	if err != nil {
		panic(errors.New("Failed to start client: " + err.Error()))
	}
	return &MerkleSquareClient{c, ctx}
}

func (mc *MerkleSquareClient) Set(key string, value string) error {
	putKey := []byte(key)
	putValue := []byte(value)

	masterSK, masterVK := crypto.GenerateKeypair()
	_, err := mc.c.Register(mc.ctx, putKey, masterSK, masterVK)
	if err != nil && err.Error() != "rpc error: code = Unknown desc = User is already registered" {
		return err
	}
	_, _, err = mc.c.Append(mc.ctx, putKey, putValue)
	if err != nil && err.Error() == "masterkey does not exist" {
		panic("System error! Retry the experiment!")
	}
	return err
}

func (mc *MerkleSquareClient) Get(key string) (string, error) {
	queryKey := []byte(key)
	val, _, err := mc.c.LookUpPK(mc.ctx, queryKey)
	if err != nil {
		return "", err
	}
	return string(val), nil
}

type ObjectId int32

var refs struct {
	sync.Mutex
	objs map[ObjectId]interface{}
	next ObjectId
}

func init() {
	refs.Lock()
	defer refs.Unlock()

	refs.objs = make(map[ObjectId]interface{})
	refs.next = 1000
}

func NewObjectId(obj interface{}) ObjectId {
	refs.Lock()
	defer refs.Unlock()

	id := refs.next
	refs.next++

	refs.objs[id] = obj
	return id
}

func (id ObjectId) IsNil() bool {
	return id == 0
}

func (id ObjectId) Get() interface{} {
	refs.Lock()
	defer refs.Unlock()

	return refs.objs[id]
}

func (id *ObjectId) Free() interface{} {
	refs.Lock()
	defer refs.Unlock()

	obj := refs.objs[*id]
	delete(refs.objs, *id)
	*id = 0

	return obj
}

func main() {
	// do nothing
}
