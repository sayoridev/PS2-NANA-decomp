# Naming Conventions

Authoritative summary (full rationale in `AGENTS.md` §8):

- Functions: `ModuleName__FunctionName` (camelCase function); unknown:
  `func_00123ABC` (address in hex, 8 digits).
- Globals: `g_CamelCase`; statics: `s_CamelCase`; unlabeled data: `D_00123ABC`.
- Structs: `ModuleName__StructName` / typedef `ModuleNameStructName`.
- Enums: `ModuleName__EnumName`, members `MODULE_ENUM_MEMBER`.
- Every reconstructed function carries a STATUS header:
  `VERIFIED | PARTIAL | RECONSTRUCTED | STUB` with original address and size.
