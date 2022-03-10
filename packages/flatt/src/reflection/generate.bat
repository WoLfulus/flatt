
flatc ^
  --gen-all ^
  --gen-mutable ^
  --gen-compare ^
  --gen-object-api ^
  --gen-name-strings ^
  --force-empty ^
  --force-empty-vectors ^
  --reflect-types ^
  --reflect-names ^
  --cpp ^
  --cpp-include array ^
  --cpp-std c++17 ^
  --cpp-static-reflection ^
  --filename-suffix "" ^
  -o . ^
  ./reflection.fbs
