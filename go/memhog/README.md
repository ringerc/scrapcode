# memhog

A small go program to allocate a bunch of RAM and sit on it for a while.

It writes only one byte per page to force the OS to allocate the page without
taking so long to fill it all.

Optionally asks the go runtime to release memory back to the OS after a while,
demonstrating how this affects memstats.

This is a dirty hack and the code is wretched. Don't use it as an example for
anything except, possibly, how not to do things.

This won't be effective as it stands on nodes without swap. It's intended for
creating memory pressure on k8s nodes, where k8s has historically been strongly
opposed to supporting swap so it hasn't been relevant. To make this useful in
the presence of swap (or features like RAM compression) it'll be necessary to
have some routines that sweep through the allocated memory to read it and ensure
it doesn't get paged out as LRU or is regularly paged back in.
