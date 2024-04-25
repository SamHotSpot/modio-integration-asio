# What's this all about?

While integrating the [mod.io Unreal Engine Plugin](https://github.com/modio/modio-ue) into one of our game projects,
I've encountered duplicate symbol definitions for some parts of the [asio](https://github.com/chriskohlhoff/asio)
library which is not only integrated into the mod.io native SDK but also a private dependency of Unreal's
`TraceAnalysis` module.
This lead to multiple [LNK2005](https://learn.microsoft.com/en-us/cpp/error-messages/tool-errors/linker-tools-error-lnk2005)
errors and caused our build to fail due to `fatal error LNK1169: one or more multiply defined symbols found`.

## Output log from our CI build server
```
********** BUILD COMMAND STARTED **********
Running: dotnet.exe [...] -Target="UnrealPak Win64 Development -Project=C:\Teamcity\work\GameBuild\UE5\Game\Game.uproject [...]
Log file: C:\Teamcity\work\GameBuild\UE5\Engine\Programs\AutomationTool\Saved\Logs\UBT-.txt
Creating makefile for UnrealPak (.uproject file is newer)
[...]
 ------ Building 82 action(s) started ------
[1/82] Compile [x64] PCH.Modio.cpp
[2/82] Compile [x64] Module.Modio.3.cpp
[3/82] Compile [x64] Module.Modio.6.cpp
[...]
[79/82] Compile [x64] Module.Game.56.cpp
[80/82] Compile [x64] Module.Game.60.cpp
[81/82] Link [x64] Game.exe
Module.Modio.5.cpp.obj : error LNK2005: "class std::error_category const & __cdecl asio::error::get_misc_category(void)" (?get_misc_category@error@asio@@YAAEBVerror_category@std@@XZ) already defined in Module.TraceAnalysis.cpp.obj
Module.Modio.5.cpp.obj : error LNK2005: "class std::error_category const & __cdecl asio::system_category(void)" (?system_category@asio@@YAAEBVerror_category@std@@XZ) already defined in Module.TraceAnalysis.cpp.obj
Module.Modio.6.cpp.obj : error LNK2005: "public: __cdecl asio::detail::select_reactor::select_reactor(class asio::execution_context &)" (??0select_reactor@detail@asio@@QEAA@AEAVexecution_context@2@@Z) already defined in Module.TraceAnalysis.cpp.obj
Module.Modio.6.cpp.obj : error LNK2005: "protected: virtual __cdecl asio::execution_context::service::~service(void)" (??1service@execution_context@asio@@MEAA@XZ) already defined in Module.TraceAnalysis.cpp.obj
Module.Modio.6.cpp.obj : error LNK2005: "protected: virtual __cdecl asio::io_context::service::~service(void)" (??1service@io_context@asio@@MEAA@XZ) already defined in Module.TraceAnalysis.cpp.obj
Module.Modio.6.cpp.obj : error LNK2005: "private: void __cdecl asio::detail::socket_select_interrupter::close_descriptors(void)" (?close_descriptors@socket_select_interrupter@detail@asio@@AEAAXXZ) already defined in Module.TraceAnalysis.cpp.obj
Module.Modio.6.cpp.obj : error LNK2005: "class std::error_category const & __cdecl asio::error::get_misc_category(void)" (?get_misc_category@error@asio@@YAAEBVerror_category@std@@XZ) already defined in Module.TraceAnalysis.cpp.obj
Module.Modio.6.cpp.obj : error LNK2005: "public: void __cdecl asio::detail::socket_select_interrupter::interrupt(void)" (?interrupt@socket_select_interrupter@detail@asio@@QEAAXXZ) already defined in Module.TraceAnalysis.cpp.obj
Module.Modio.6.cpp.obj : error LNK2005: "public: virtual void __cdecl asio::detail::select_reactor::notify_fork(enum asio::execution_context::fork_event)" (?notify_fork@select_reactor@detail@asio@@UEAAXW4fork_event@execution_context@3@@Z) already defined in Module.TraceAnalysis.cpp.obj
Module.Modio.6.cpp.obj : error LNK2005: "private: virtual void __cdecl asio::execution_context::service::notify_fork(enum asio::execution_context::fork_event)" (?notify_fork@service@execution_context@asio@@EEAAXW4fork_event@23@@Z) already defined in Module.TraceAnalysis.cpp.obj
Module.Modio.6.cpp.obj : error LNK2005: "private: virtual void __cdecl asio::io_context::service::notify_fork(enum asio::execution_context::fork_event)" (?notify_fork@service@io_context@asio@@EEAAXW4fork_event@execution_context@3@@Z) already defined in Module.TraceAnalysis.cpp.obj
Module.Modio.6.cpp.obj : error LNK2005: "private: void __cdecl asio::detail::socket_select_interrupter::open_descriptors(void)" (?open_descriptors@socket_select_interrupter@detail@asio@@AEAAXXZ) already defined in Module.TraceAnalysis.cpp.obj
Module.Modio.6.cpp.obj : error LNK2005: "public: unsigned __int64 __cdecl asio::detail::scheduler::poll_one(class std::error_code &)" (?poll_one@scheduler@detail@asio@@QEAA_KAEAVerror_code@std@@@Z) already defined in Module.TraceAnalysis.cpp.obj
Module.Modio.6.cpp.obj : error LNK2005: "public: virtual void __cdecl asio::detail::scheduler::shutdown(void)" (?shutdown@scheduler@detail@asio@@UEAAXXZ) already defined in Module.TraceAnalysis.cpp.obj
Module.Modio.6.cpp.obj : error LNK2005: "public: virtual void __cdecl asio::detail::select_reactor::shutdown(void)" (?shutdown@select_reactor@detail@asio@@UEAAXXZ) already defined in Module.TraceAnalysis.cpp.obj
Module.Modio.6.cpp.obj : error LNK2005: "private: virtual void __cdecl asio::io_context::service::shutdown(void)" (?shutdown@service@io_context@asio@@EEAAXXZ) already defined in Module.TraceAnalysis.cpp.obj
Module.Modio.6.cpp.obj : error LNK2005: "public: virtual void __cdecl asio::detail::strand_executor_service::shutdown(void)" (?shutdown@strand_executor_service@detail@asio@@UEAAXXZ) already defined in Module.TraceAnalysis.cpp.obj
Module.Modio.6.cpp.obj : error LNK2005: "class std::error_category const & __cdecl asio::system_category(void)" (?system_category@asio@@YAAEBVerror_category@std@@XZ) already defined in Module.TraceAnalysis.cpp.obj
   Creating library C:\Teamcity\work\GameBuild\UE5\Game\Binaries\Win64\Game.lib and object C:\Teamcity\work\GameBuild\UE5\Game\Binaries\Win64\Game.exp
C:\Teamcity\work\GameBuild\UE5\Game\Binaries\Win64\Game.exe : fatal error LNK1169: one or more multiply defined symbols found
```

## Now what?
After a lot of poking around in the bits and pieces that make up our rather vast code base, the only working solution
I could find is inspired by the idea behind the blog entry
[Avoiding Static Linking Collisions in C++ with Dynamic Namespacing](https://spin.atomicobject.com/static-linking-c-plus-plus/)
written by [Greg Williams](https://spin.atomicobject.com/author/williams/) back in 2014.

The basic concept is that you re-define the namespace identifier of the library you use before including its header file(s).
So it boils down to the following:
```
#include "myheader.h"
#define FooNamespace MyCustomFooNamespace
#include "fooLib/FooLibrary.h"
...
static int main(void)
{
  FooNamespace::Foo regular_foo;
  MyCustomFooNamespace::Foo custom_foo;
  ...
}
```

While this didn't exactly work out due to me being unable to tell when and where all the corresponding #include
directives are located within the mod.io SDK, it still pointed me in the direction of another approach which
involves lots of [_search and replace_ using Visual Studio Code](https://stackoverflow.com/a/41270417),
but in the end this actually works.

The gist of this is: replace every occurrence of `namespace asio` or `asio::` with another identifier,
`ASIO_LIBNS` for example and let the compiler redefine this when compiling the code base via definition
`-DASIO_LIBNS=custom_asio`.

### ℹ️ Note
Thinking about it, this _should_ work out of the box by simply using the identical mechanism but
replacing `asio` instead, so `-Dasio=custom_asio` _should_ lead to the same result although it never
worked for me.🤷
