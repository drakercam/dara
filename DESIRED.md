#Feature						Difficulty					Notes
construct definitions		| Medium |			Field metadata, function metadata, instance creation
Designated initialization	| Medium |			Player{.x = 1, .y = 1}
Construct field access		| Medium |			Chained expressions like game.player.x
Controlled mutation			| Medium–high |		Track the owning construct and validate writes
while and logical operators	| Low–medium |		Parser precedence and execution flow
@private annotations		| Medium | 			Function visibility and access checks
import and from ... import	| Medium–high |		Module loading, symbol resolution, caching
define raylib as rl			| Medium–high |		Native module registration and aliases
Native C bindings			| Medium–high |		Argument conversion, types, errors, return values
std.dara					| Medium |			Built on top of the other features
Hot reloading				| Medium–high |		Module lifecycle and state preservation
