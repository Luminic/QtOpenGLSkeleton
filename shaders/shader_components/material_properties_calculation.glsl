{
  roughness = material.roughness;
  if (material.use_roughness_map) {
    roughness *= length(texture(material.roughness_map, fs_in.texture_coordinate).rgb)/1.73f;
  }

  shininess = pow(2,(roughness)*10);

  metalness = material.metalness;
  if (material.use_metalness_map) {
    metalness *= length(texture(material.metalness_map, fs_in.texture_coordinate).rgb)/1.73f;
  }

  color = material.color;
  if (material.use_albedo_map) {
    color *= texture(material.albedo_map, fs_in.texture_coordinate).rgb;
  }
  diffuse = material.diffuse * color;

  specular = vec3(material.specular);
  specular *= pow(roughness, 2);

  metal_tint = color;
  if (metalness >= 0.9f) {
    specular *= normalize(diffuse) * 1.73;
    diffuse *= 1.0f-roughness;
  }

  ambient = color * material.ambient;
  if (material.use_ambient_occlusion_map) {
    ambient *= texture(material.ambient_occlusion_map, fs_in.texture_coordinate).rgb;
  }
}
