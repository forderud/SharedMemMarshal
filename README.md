Sample code for custom COM marshaling with shared memory.

Goal:
* Leverage COM runtime to keep server object alive until all associated proxies have been destroyed.
* How to establish a "strong connection" between proxy & server when implemenging IMarshal?

Challenge:
* Must call `AddRef()` in [`IMarshal::MarshalInterface`](https://docs.microsoft.com/nb-no/windows/desktop/api/objidl/nf-objidl-imarshal-marshalinterface) to avoid premature server object destruction. This introduces a reference leak. I'm unable to find a corresponding place to put a `Release()` to avoid leaking without having to introduce complex IPC schemes that rely on event signaling to manage lifetime between proxy and server.

References:
* [IMarhal](https://docs.microsoft.com/nb-no/windows/desktop/api/objidl/nn-objidl-imarshal) interface
* Inside COM+: [Will That Be Custom or Standard Marshaling?](https://thrysoee.dk/InsideCOM+/ch14c.htm) - uses event objects to synchronize destruction from proxy to server. This require active event polling by the server and is not will cause a leak if the proxy crashes.
