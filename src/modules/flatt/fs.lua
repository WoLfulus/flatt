local fs = {}

function fs.exists(...)
  return flatt_fs_exists(...)
end

function fs.is_dir(...)
  return flatt_fs_is_dir(...)
end

function fs.is_file(...)
  return flatt_fs_is_file(...)
end

function fs.read_file(...)
  return flatt_fs_read_file(...)
end

function fs.remove_file(...)
  return flatt_fs_remove_file(...)
end

function fs.is_file(...)
  return flatt_fs_is_file(...)
end

function fs.write_file(...)
  return flatt_fs_write_file(...)
end

function fs.write_json(...)
  return flatt_fs_write_json(...)
end

function fs.hash_dir(...)
  return flatt_fs_hash_dir(...)
end

function fs.hash_file(...)
  return flatt_fs_hash_file(...)
end

function fs.hash(...)
  return flatt_fs_hash(...)
end

function fs.list_files(...)
  return flatt_fs_list_files(...)
end

function fs.list_dirs(...)
  return flatt_fs_list_dirs(...)
end

function fs.list(...)
  return flatt_fs_list(...)
end

return fs
