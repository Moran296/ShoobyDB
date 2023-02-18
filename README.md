# ShoobyDB
Lightweight configurations database for real time embedded systems in c++20


## USER REGISTERED BLOBS

This feature lets user to register blobs as possible blobs to the metadata.

It does this by letting user add types to the variant of the meta data.
This is safer and gives more accurate visitor abilities to the user.

But it also makes the metadata struct that is held per configuration (blob or not) bigger
Because the variant might hold a blob that is bigger than the size of a pointer.

It also has some Limitions, the blob that is registered must be constexpr constructable
