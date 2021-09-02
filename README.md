Sample code for custom COM marshaling with shared memory.

## IPC mechanism
Must call `AddRef()` in [`IMarshal::MarshalInterface`](https://docs.microsoft.com/nb-no/windows/desktop/api/objidl/nf-objidl-imarshal-marshalinterface) to avoid premature server object destruction before the proxy object is created in the client process. This triggers the need for implementing a IPC mechanism to signal proxy destruction back to the server, so that the server can call `Release()`.

The implementation uses Windows [event objects](https://docs.microsoft.com/nb-no/windows/desktop/Sync/event-objects) as IPC mechanism.

## DCOM garbage collector problem

Associated StackOverflow question: https://stackoverflow.com/questions/69010789/how-to-leverage-dcom-garbage-collector-with-custom-marshaling-imarshal

A client process that leaks COM references due to bugs or a crash will lead to leaking stubs in the server for the `DataHandle` class that implements `IMarhal`. These leaks are not cleaned up by the DCOM garbage collector. Leaking `DataCollection` class references on the other hand are cleaned up automatically. It would be nice of the COM runtime could somwehow also clean up for classes implementing IMarhal.

## Steps to reproduce the problem

Test to prove that `DataCollection` leaks are automatically cleaned up:
1. Add `mgr.p->AddRef();` somwhere in `int main()` to provoke a `DataCollection` reference leak.
2. Build solution and start TestClient.exe in debug mode.
3. Observe that TestServer.exe is started and terminates a few seconds _after_ TestClient.exe terminates.

Test to prove that `DataHandle` leaks are _not_ cleaned up:
1. Comment-out the `m_signal.Signal()` line in `DataHandleProxy::InternalRelease()` to provoke a `DataHandle` reference leak.
2. Build solution and start TestClient.exe in debug mode.
3. Observe that TestServer.exe is started and continues to live also _after_ TestClient.exe terminates.


### References
* [IMarhal](https://docs.microsoft.com/nb-no/windows/desktop/api/objidl/nn-objidl-imarshal) interface
* Inside COM+: [Will That Be Custom or Standard Marshaling?](https://thrysoee.dk/InsideCOM+/ch14c.htm) - uses event objects to synchronize destruction from proxy to server.
