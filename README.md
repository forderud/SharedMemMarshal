Sample code for custom COM marshaling with shared memory.

## Custom COM marshaling details
Must call `AddRef()` in [`IMarshal::MarshalInterface`](https://docs.microsoft.com/nb-no/windows/desktop/api/objidl/nf-objidl-imarshal-marshalinterface) to avoid premature server object destruction before the proxy object is created in the client process. This triggers the need for implementing a IPC mechanism to signal proxy destruction back to the server, so that the server can later call `Release()` on itself to clean up.

The implementation uses Windows [event objects](https://docs.microsoft.com/nb-no/windows/desktop/Sync/event-objects) as IPC mechanism.

## DCOM garbage collector limitation

Associated StackOverflow question: https://stackoverflow.com/questions/69010789/how-to-leverage-dcom-garbage-collector-with-custom-marshaling-imarshal (answered)

The DCOM garbage collector usually cleans up leaking stub references after client processes terminate. This automatic garbage collection is unfortunately not available for classes implementing IMarshal. This limitation can be worked around by introducing an extra COM object for maintaining references from the client-side proxy back to the server. This is implemented using a tiny `RefOwner` class that is marshalled using `CoMarshalInterface`/`CoUnmarshalInterface` from the server to the proxy.

## References
* [IMarhal](https://docs.microsoft.com/nb-no/windows/desktop/api/objidl/nn-objidl-imarshal) interface
* Inside COM+: [Will That Be Custom or Standard Marshaling?](https://thrysoee.dk/InsideCOM+/ch14c.htm) - uses event objects to synchronize destruction from proxy to server.
