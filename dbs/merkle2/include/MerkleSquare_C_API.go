package main

//#include "./merklesquare_capi.h"
import "C"
import "unsafe"

//export merklesquareclient_new
func merklesquareclient_new(serverAddr, auditorAddr, verifierAddr *C.char) C.merklesquareclient_handle_t {
	id := NewObjectId(NewMerkleSquareClient(C.GoString(serverAddr), C.GoString(auditorAddr), C.GoString(verifierAddr)))
	return C.merklesquareclient_handle_t(id)
}

//export merklesquareclient_delete
func merklesquareclient_delete(h C.merklesquareclient_handle_t) {
	// id := ObjectId(uintptr(unsafe.Pointer(h)))
	id := ObjectId(h)
	id.Free()
}

//export merklesquareclient_set
func merklesquareclient_set(h C.merklesquareclient_handle_t, key *C.char, value *C.char) C.int {
	// p := ObjectId(uintptr(unsafe.Pointer(h))).Get().(*MerkleSquareClient)
	p := ObjectId(h).Get().(*MerkleSquareClient)
	err := p.Set(C.GoString(key), C.GoString(value))
	if err != nil {
		return C.int(1)
	}
	return C.int(0)
}

//export merklesquareclient_get
func merklesquareclient_get(h C.merklesquareclient_handle_t, key *C.char, buf *C.char) C.int {
	// p := ObjectId(uintptr(unsafe.Pointer(h))).Get().(*MerkleSquareClient)
	p := ObjectId(h).Get().(*MerkleSquareClient)
	value, err := p.Get(C.GoString(key))
	if err != nil {
		if err.Error() == "rpc error: code = Unknown desc = no user key found" {
			return C.int(1)
		}
		return C.int(0)
	}
	n := len(value)
	bufSlice := ((*[1 << 31]byte)(unsafe.Pointer(buf)))[0:n:n]
	n = copy(bufSlice, []byte(value))
	return C.int(0)
}
