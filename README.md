# rapscallion
C++ RPC using C++17 futures

# Why?
Most existing RPC mechanisms have two modes to operate in. Either you have no return path for replies (asynchronous), or you do have one and you block the receiving end until it has received everything (synchronous). Both of these have major downsides. 

Asynchronous means that you have to match up whatever data you get back yourselves. For one-way unreliable communication this is fine up to a point, but it becomes hard to manage for any kind of reliability.

Synchronous is better, but looks very close to in-language constructs and has the danger that people do not realize that such an RPC call can take a long time or may disconnect. Handling disconnections is an even bigger topic, where exceptions are probably the best solution but that again implies that you may get exceptions from functions that would otherwise not throw exceptions - a clear difference between having a local or a remote connection.

Rapscallion takes the technical solution of using futures, which is a way to run tasks "asynchronously", and applies it to remote execution of function calls - remote procedure calls. Any future can throw an exception, and the path for them is fully defined. Using the C++17 proposal for combining futures it is possible to program a distributed application fully asynchronously, but with continuations.

In other words, it's like the RPC you knew, except without the trouble.

# Requirements

* MUST use only standard C++11
 - MUST NOT rely on later standards
 - MUST NOT use compiler/language extensions
* MUST support promise pipelining and SHOULD do so as transparently as possible
* SHOULD have a pure C++ IDL (without relying on helper macros/templates):
 - This SHOULD be compatible as much as possible with static reflection, it MUST NOT contradict the `$reflect` proposal, but MAY rely on missing features ([p0385r2](http://www.open-std.org/Jtc1/sc22/wg21/docs/papers/2017/p0385r2.pdf))
 - A Clang-based code generator SHOULD be provided for C++11 compatibility
* MUST have an IDL expressible completely in C++11 and preprocessor support macros
 - MUST NOT require a code generator for serialization
 - SHOULD NOT require a code generator for RPC proxies
* MUST support serialization of simple structs and its by-value members, _without_ any manual support code
* Supporting by-reference members (pointers, references) MAY work only if serialization-support code is provided
* Interfaces are a collection a of functions
 - [TODO]: Maybe only allow functions outside of class scope? I.e. ADL lookup instead of special syntax for object orientation?
 - This would make generating the client-side proxy functions easier because no future-object-proxy would be necessary to support transparent promise pipelining
* Types that MUST be serializable out of the box:
 - `int8_t`, `uint8_t`, `int16_t`, `uint16_t`, `int32_t`, `uint32_t`, `int64_t`, `uint64_t`
 - std::string, std::u8string
 - `std::vector<T>`, `std::map<K, V>`, `std::list<T>`, `std::deque<T>`, `std::set<T>` for every type `T`, `K` and `V` that's serializable
* The specific serialization format used should be interchangable via a policy
* The default base serialization format MUST, for POD-types, be equal to that of the C++ ABI on Linux x86-64.
 - This enables using shared memory as a transport layer
 - This enables direct use of to-send data for transmission and receive buffers as decoded data when bandwidth isn't an issue
* On top of that a serialization packing algorithm SHOULD be defined to be more space efficient for cases where size is an issue
 - That packing algorithm SHOULD be limited to eliminating padding, eliding unused (optional) fields and using novel encodings of integers
 - It MUST NOT be a generic compression algorithm, that can be layered on top if so desired.
* The transport layer MUST be interchangable
 - Unless the transport and serialization components collaborate the transport layer MUST support framing

# Design

## Protocol

* Both endpoints MUST maintain a request ID and reply ID table
 - These IDs should be natural numbers ranging from 0-2^32-2. The ID value 2^32-1 is reserved as a sentinel value and SHOULD NOT be transmitted over the wire.
 - For each ID an internal reference count and a "net-release" flag MUST be maintained
  + The internal reference count describes the amount of references existing within the program to this ID
  + The "net-release" flag is a boolean, indicating that the remote reference count has dropped to zero and that the remote shall never use it again to refer to the same request
 - When initiating a new request the sender allocates a new ID in its own request ID table
  + This SHOULD be the lowest unused ID number
  + It MUST set the net-release flag to `false`
  + It MUST have at least one internal reference (or transmission of the request would be pointless)
 - It MUST include this number as part of its transmitted "CALL" request
 - Additionally, in its request, it must indicate:
  + whether or not it wishes to receive the computed result as soon as it's available; and
  + whether or not it wishes the result to be retained by the receiver, until either:
   - the connection is dropped; or
   - the sender sends a "FINISH" or "RELEASE" request with a matching request ID
 - Upon reception of a request, the receiver adds the sender's request ID to its own reply ID
  + The ID should be numerically equal to the matching request ID
  + It MUST set the net-release flag to `false`
