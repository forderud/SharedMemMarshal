Sample code for custom COM marshaling with shared memory.

### Main challenge
Must call `AddRef()` in [`IMarshal::MarshalInterface`](https://docs.microsoft.com/nb-no/windows/desktop/api/objidl/nf-objidl-imarshal-marshalinterface) to avoid premature server object destruction before the proxy object is created. This triggers the need for implementing a IPC mechanism to signal proxy destruction back to the server, so that the server can call `Release()`.


### Questions
* Possible to leverage the COM runtime to keep server object alive until all associated proxies have been destroyed?
* Possible to establish a "strong connection" between proxy & server when implementing IMarshal?

### References
* [IMarhal](https://docs.microsoft.com/nb-no/windows/desktop/api/objidl/nn-objidl-imarshal) interface
* Inside COM+: [Will That Be Custom or Standard Marshaling?](https://thrysoee.dk/InsideCOM+/ch14c.htm) - uses event objects to synchronize destruction from proxy to server. This require active event polling by the server and is not will cause a leak if the proxy crashes.
