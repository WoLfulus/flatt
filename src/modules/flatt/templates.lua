local templates = {}

local json = rock("lunajson")

function templates.render_json(template, data)
  return flatt_template_render(template, data)
end

function templates.render(template, data)
  if type(data) == "table" then
    return templates.render_json(template, json.encode(data))
  elseif type(data) == "string" then
    return templates.render_json(template, data)
  else
    error("template data format not supported: "..type(data))
  end
end

return templates
