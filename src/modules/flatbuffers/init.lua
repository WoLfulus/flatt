local flatbuffers = {}

function flatbuffers.compile(...)
  return flatbuffers_compile(...)
end

function flatbuffers.reflect(...)
  return flatbuffers_reflect(...)
end

return flatbuffers
