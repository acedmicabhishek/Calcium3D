#include "ModelImporter.h"
#include "../Core/Logger.h"
#include "../Core/ResourceManager.h"

#include <tiny_obj_loader.h>
#include <ufbx/ufbx.h>
#include <nlohmann/json.hpp>

#include <fstream>
#include <sstream>
#include <algorithm>
#include <filesystem>
#include <cstring>
#include <cmath>
#include <functional>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

namespace fs = std::filesystem;

static glm::mat4 ToMat4(const ufbx_matrix& m) {
    glm::mat4 out;
    out[0] = { (float)m.cols[0].x, (float)m.cols[0].y, (float)m.cols[0].z, 0.0f };
    out[1] = { (float)m.cols[1].x, (float)m.cols[1].y, (float)m.cols[1].z, 0.0f };
    out[2] = { (float)m.cols[2].x, (float)m.cols[2].y, (float)m.cols[2].z, 0.0f };
    out[3] = { (float)m.cols[3].x, (float)m.cols[3].y, (float)m.cols[3].z, 1.0f };
    return out;
}

std::vector<std::string> ModelImporter::GetSupportedExtensions() {
    return { ".obj", ".fbx", ".gltf", ".glb", ".stl", ".ply" };
}

bool ModelImporter::IsModelFile(const std::string& extension) {
    std::string ext = extension;
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
    for (auto& supported : GetSupportedExtensions()) {
        if (ext == supported) return true;
    }
    return false;
}

ImportResult ModelImporter::Import(const std::string& filepath) {
    std::string ext = fs::path(filepath).extension().string();
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

    Logger::AddLog("[ModelImporter] Importing: %s", filepath.c_str());

    ImportResult result;
    result.sourceFile = filepath;

    if (!fs::exists(filepath)) {
        result.error = "File not found: " + filepath;
        Logger::AddLog("[ModelImporter] ERROR: %s", result.error.c_str());
        return result;
    }

    if (ext == ".obj")       result = ImportOBJ(filepath);
    else if (ext == ".fbx")  result = ImportFBX(filepath);
    else if (ext == ".gltf" || ext == ".glb") result = ImportGLTF(filepath);
    else if (ext == ".stl")  result = ImportSTL(filepath);
    else if (ext == ".ply")  result = ImportPLY(filepath);
    else {
        result.error = "Unsupported format: " + ext;
        Logger::AddLog("[ModelImporter] ERROR: %s", result.error.c_str());
    }

    if (result.success) {
        Logger::AddLog("[ModelImporter] Success! Loaded %zu mesh(es) from %s",
            result.meshes.size(), fs::path(filepath).filename().c_str());
    }

    return result;
}




ImportResult ModelImporter::ImportOBJ(const std::string& filepath) {
    ImportResult result;
    result.sourceFile = filepath;

    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warn, err;

    std::string baseDir = fs::path(filepath).parent_path().string() + "/";

    if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, filepath.c_str(), baseDir.c_str())) {
        result.error = "OBJ load failed: " + warn + err;
        Logger::AddLog("[ModelImporter] %s", result.error.c_str());
        return result;
    }

    if (!warn.empty()) Logger::AddLog("[ModelImporter] OBJ Warning: %s", warn.c_str());

    for (const auto& shape : shapes) {
        ImportedMeshData mesh;
        mesh.name = shape.name.empty() ? fs::path(filepath).stem().string() : shape.name;

        for (const auto& index : shape.mesh.indices) {
            Vertex vertex;

            vertex.position = {
                attrib.vertices[3 * index.vertex_index + 0],
                attrib.vertices[3 * index.vertex_index + 1],
                attrib.vertices[3 * index.vertex_index + 2]
            };

            if (index.normal_index >= 0 && 3 * index.normal_index + 2 < (int)attrib.normals.size()) {
                vertex.normal = {
                    attrib.normals[3 * index.normal_index + 0],
                    attrib.normals[3 * index.normal_index + 1],
                    attrib.normals[3 * index.normal_index + 2]
                };
            } else {
                vertex.normal = glm::vec3(0.0f, 1.0f, 0.0f);
            }

            if (index.texcoord_index >= 0 && 2 * index.texcoord_index + 1 < (int)attrib.texcoords.size()) {
                vertex.texUV = {
                    attrib.texcoords[2 * index.texcoord_index + 0],
                    attrib.texcoords[2 * index.texcoord_index + 1]
                };
            } else {
                vertex.texUV = glm::vec2(0.0f);
            }

            vertex.color = glm::vec3(1.0f);

            mesh.indices.push_back(static_cast<GLuint>(mesh.vertices.size()));
            mesh.vertices.push_back(vertex);
        }

        if (!shape.mesh.material_ids.empty() && shape.mesh.material_ids[0] >= 0) {
            int matIdx = shape.mesh.material_ids[0];
            if (matIdx < (int)materials.size()) {
                auto& mat = materials[matIdx];
                mesh.albedo = glm::vec3(mat.diffuse[0], mat.diffuse[1], mat.diffuse[2]);

                if (!mat.diffuse_texname.empty()) {
                    std::string texPath = baseDir + mat.diffuse_texname;
                    if (fs::exists(texPath)) {
                        mesh.textures.push_back(Texture(texPath.c_str(), "diffuse"));
                    }
                }
            }
        }

        result.meshes.push_back(std::move(mesh));
    }

    result.success = true;
    return result;
}




ImportResult ModelImporter::ImportFBX(const std::string& filepath) {
    ImportResult result;
    result.sourceFile = filepath;

    ufbx_load_opts opts = {};
    opts.target_axes = ufbx_axes_right_handed_y_up;
    opts.target_unit_meters = 1.0f;

    ufbx_error error;
    ufbx_scene* scene = ufbx_load_file(filepath.c_str(), &opts, &error);
    if (!scene) {
        char errBuf[512];
        snprintf(errBuf, sizeof(errBuf), "FBX load failed: %s", error.description.data);
        result.error = errBuf;
        Logger::AddLog("[ModelImporter] %s", result.error.c_str());
        return result;
    }

    


    std::string modelDir = fs::path(filepath).parent_path().string() + "/";

    
    auto findTexture = [&](const std::string& rawPath) -> std::string {
        if (fs::exists(rawPath)) return rawPath;
        
        std::string fname = fs::path(rawPath).filename().string();
        
        
        std::string p1 = modelDir + fname;
        if (fs::exists(p1)) return p1;
        
        
        std::string p2 = modelDir + "textures/" + fname;
        if (fs::exists(p2)) return p2;
        
        
        std::string p3 = modelDir + "Textures/" + fname;
        if (fs::exists(p3)) return p3;
        
        return "";
    };

    
    for (size_t ni = 0; ni < scene->nodes.count; ni++) {
        ufbx_node* node = scene->nodes.data[ni];
        if (!node->mesh) continue;
        
        ufbx_mesh* umesh = node->mesh;
        
        ufbx_matrix xform = node->geometry_to_world;
        ufbx_matrix norm_xform = ufbx_matrix_for_normals(&xform);

        ImportedMeshData mesh;
        std::string nodeName = node->name.length > 0 
            ? std::string(node->name.data, node->name.length)
            : (umesh->name.length > 0 ? std::string(umesh->name.data, umesh->name.length) 
                                      : "Mesh_" + std::to_string(ni));
        mesh.name = nodeName;

        size_t numTriIndices = umesh->max_face_triangles * 3;
        std::vector<uint32_t> triIndices(numTriIndices);

        for (size_t fi = 0; fi < umesh->faces.count; fi++) {
            ufbx_face face = umesh->faces.data[fi];
            uint32_t numTris = ufbx_triangulate_face(triIndices.data(), triIndices.size(), umesh, face);

            for (uint32_t ti = 0; ti < numTris * 3; ti++) {
                uint32_t idx = triIndices[ti];

                Vertex vertex;

                
                ufbx_vec3 localPos = ufbx_get_vertex_vec3(&umesh->vertex_position, idx);
                ufbx_vec3 worldPos = ufbx_transform_position(&xform, localPos);
                vertex.position = glm::vec3(worldPos.x, worldPos.y, worldPos.z);

                if (umesh->vertex_normal.exists) {
                    ufbx_vec3 localNorm = ufbx_get_vertex_vec3(&umesh->vertex_normal, idx);
                    ufbx_vec3 worldNorm = ufbx_transform_direction(&norm_xform, localNorm);
                    
                    glm::vec3 n(worldNorm.x, worldNorm.y, worldNorm.z);
                    float len = glm::length(n);
                    vertex.normal = len > 0.0001f ? n / len : glm::vec3(0.0f, 1.0f, 0.0f);
                } else {
                    vertex.normal = glm::vec3(0.0f, 1.0f, 0.0f);
                }

                if (umesh->vertex_uv.exists) {
                    ufbx_vec2 uv = ufbx_get_vertex_vec2(&umesh->vertex_uv, idx);
                    vertex.texUV = glm::vec2(uv.x, uv.y);
                } else {
                    vertex.texUV = glm::vec2(0.0f);
                }

                vertex.color = glm::vec3(1.0f);

                


                mesh.indices.push_back(static_cast<GLuint>(mesh.vertices.size()));
                mesh.vertices.push_back(vertex);
            }
        }
        
        


        
        if (umesh->materials.count > 0 && umesh->materials.data[0]) {
            ufbx_material* mat = umesh->materials.data[0];
            
            if (mat->fbx.diffuse_color.has_value) {
                ufbx_vec4 dc = mat->fbx.diffuse_color.value_vec4;
                mesh.albedo = glm::vec3(dc.x, dc.y, dc.z);
            }

            
            auto tryLoadTexture = [&](ufbx_texture* tex) -> std::string {
                if (!tex) return "";
                
                
                std::string searchName;
                
                if (tex->relative_filename.length > 0) {
                    std::string rel(tex->relative_filename.data, tex->relative_filename.length);
                    
                    std::replace(rel.begin(), rel.end(), '\\', '/');
                    searchName = fs::path(rel).filename().string();
                }
                if (searchName.empty() && tex->filename.length > 0) {
                    std::string full(tex->filename.data, tex->filename.length);
                    std::replace(full.begin(), full.end(), '\\', '/');
                    searchName = fs::path(full).filename().string();
                }
                
                if (searchName.empty()) return "";
                
                
                std::string searchLower = searchName;
                std::transform(searchLower.begin(), searchLower.end(), searchLower.begin(), ::tolower);
                
                
                std::vector<std::string> searchDirs = {
                    modelDir,
                    fs::path(modelDir).parent_path().string() + "/",
                    modelDir + "textures/",
                    modelDir + "Textures/",
                    fs::path(modelDir).parent_path().string() + "/textures/",
                    fs::path(modelDir).parent_path().string() + "/Textures/"
                };
                
                for (auto& dir : searchDirs) {
                    if (!fs::exists(dir) || !fs::is_directory(dir)) continue;
                    for (auto& entry : fs::directory_iterator(dir)) {
                        if (!entry.is_regular_file()) continue;
                        std::string fname = entry.path().filename().string();
                        std::string fnameLower = fname;
                        std::transform(fnameLower.begin(), fnameLower.end(), fnameLower.begin(), ::tolower);
                        
                        if (fnameLower == searchLower) {
                            Logger::AddLog("[ModelImporter] Found texture: %s -> %s", 
                                searchName.c_str(), entry.path().string().c_str());
                            return entry.path().string();
                        }
                    }
                }
                
                Logger::AddLog("[ModelImporter] Texture not found: %s", searchName.c_str());
                return "";
            };

            
            std::string diffPath;
            if (mat->fbx.diffuse_color.texture) {
                diffPath = tryLoadTexture(mat->fbx.diffuse_color.texture);
            }
            
            if (diffPath.empty() && mat->pbr.base_color.texture) {
                diffPath = tryLoadTexture(mat->pbr.base_color.texture);
            }
            
            if (!diffPath.empty()) {
                mesh.textures.push_back(Texture(diffPath.c_str(), "diffuse"));
                
                mesh.albedo = glm::vec3(1.0f);
            }
        }

        if (!mesh.vertices.empty()) {
            result.meshes.push_back(std::move(mesh));
        }
    }

    


    ufbx_free_scene(scene);
    result.success = !result.meshes.empty();
    if (!result.success) result.error = "No meshes found in FBX file";
    return result;
}




ImportResult ModelImporter::ImportGLTF(const std::string& filepath) {
    ImportResult result;
    result.sourceFile = filepath;

    std::string ext = fs::path(filepath).extension().string();
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

    nlohmann::json gltf;
    std::vector<unsigned char> binData;
    std::string baseDir = fs::path(filepath).parent_path().string() + "/";

    if (ext == ".glb") {
        std::ifstream file(filepath, std::ios::binary);
        if (!file) { result.error = "Cannot open GLB file"; return result; }

        uint32_t magic, version, length;
        file.read(reinterpret_cast<char*>(&magic), 4);
        file.read(reinterpret_cast<char*>(&version), 4);
        file.read(reinterpret_cast<char*>(&length), 4);

        if (magic != 0x46546C67) { result.error = "Not a valid GLB file"; return result; }

        uint32_t jsonChunkLen, jsonChunkType;
        file.read(reinterpret_cast<char*>(&jsonChunkLen), 4);
        file.read(reinterpret_cast<char*>(&jsonChunkType), 4);

        std::string jsonStr(jsonChunkLen, '\0');
        file.read(&jsonStr[0], jsonChunkLen);
        gltf = nlohmann::json::parse(jsonStr);

        if (file.tellg() < (std::streampos)length) {
            uint32_t binChunkLen, binChunkType;
            file.read(reinterpret_cast<char*>(&binChunkLen), 4);
            file.read(reinterpret_cast<char*>(&binChunkType), 4);
            binData.resize(binChunkLen);
            file.read(reinterpret_cast<char*>(binData.data()), binChunkLen);
        }
    } else {
        std::ifstream file(filepath);
        if (!file) { result.error = "Cannot open glTF file"; return result; }
        file >> gltf;
        if (gltf.contains("buffers") && !gltf["buffers"].empty()) {
            for (auto& buffer : gltf["buffers"]) {
                if (buffer.contains("uri")) {
                    std::string uri = buffer["uri"];
                    std::ifstream binFile(baseDir + uri, std::ios::binary);
                    if (binFile) {
                        std::vector<unsigned char> data((std::istreambuf_iterator<char>(binFile)), std::istreambuf_iterator<char>());
                        binData.insert(binData.end(), data.begin(), data.end());
                    }
                }
            }
        }
    }

    auto getFloats = [&](const nlohmann::json& accessor) -> std::vector<float> {
        if (accessor.is_null()) return {};
        unsigned int bvIdx = accessor.value("bufferView", 0);
        unsigned int count = accessor["count"];
        unsigned int byteOff = accessor.value("byteOffset", 0);
        std::string type = accessor["type"];
        unsigned int compType = accessor.value("componentType", 5126);

        auto& bv = gltf["bufferViews"][bvIdx];
        unsigned int bvOff = bv.value("byteOffset", 0);
        unsigned int byteStride = bv.value("byteStride", 0);

        unsigned int numPer = 1;
        if (type == "VEC2") numPer = 2;
        else if (type == "VEC3") numPer = 3;
        else if (type == "VEC4") numPer = 4;

        if (byteStride == 0) {
            unsigned int compSize = (compType == 5126) ? 4 : (compType == 5123 ? 2 : 1);
            byteStride = numPer * compSize;
        }

        std::vector<float> out;
        unsigned int start = bvOff + byteOff;
        bool normalized = accessor.value("normalized", false);
        
        for (unsigned int i = 0; i < count; i++) {
            unsigned int elementStart = start + i * byteStride;
            for (unsigned int j = 0; j < numPer; j++) {
                if (compType == 5126) {
                    float val;
                    std::memcpy(&val, &binData[elementStart + j * 4], 4);
                    out.push_back(val);
                } else if (compType == 5123) {
                    uint16_t val;
                    std::memcpy(&val, &binData[elementStart + j * 2], 2);
                    out.push_back(normalized ? (val / 65535.0f) : (float)val);
                } else if (compType == 5121) {
                    uint8_t val = binData[elementStart + j];
                    out.push_back(normalized ? (val / 255.0f) : (float)val);
                } else {
                    out.push_back(0.0f);
                }
            }
        }
        return out;
    };

    

    auto getIndices = [&](const nlohmann::json& accessor) -> std::vector<GLuint> {
        if (accessor.is_null()) return {};
        unsigned int bvIdx = accessor.value("bufferView", 0);
        unsigned int count = accessor["count"];
        unsigned int byteOff = accessor.value("byteOffset", 0);
        unsigned int compType = accessor["componentType"];

        auto& bv = gltf["bufferViews"][bvIdx];
        unsigned int bvOff = bv.value("byteOffset", 0);
        unsigned int start = bvOff + byteOff;

        std::vector<GLuint> out;
        if (compType == 5125) {
            for (unsigned int i = 0; i < count; i++) {
                uint32_t v; std::memcpy(&v, &binData[start + i * 4], 4);
                out.push_back(v);
            }
        } else if (compType == 5123) {
            for (unsigned int i = 0; i < count; i++) {
                uint16_t v; std::memcpy(&v, &binData[start + i * 2], 2);
                out.push_back(v);
            }
        } else if (compType == 5121) {
            for (unsigned int i = 0; i < count; i++) {
                out.push_back(binData[start + i]);
            }
        }
        return out;
    };

    if (!gltf.contains("meshes")) {
        result.error = "glTF has no meshes";
        return result;
    }

    auto processMesh = [&](int meshIdx, const glm::mat4& transform) {
        if (meshIdx >= (int)gltf["meshes"].size()) return;
        auto& gMesh = gltf["meshes"][meshIdx];
        std::string meshName = gMesh.value("name", "Mesh_" + std::to_string(meshIdx));

        for (size_t pi = 0; pi < gMesh["primitives"].size(); pi++) {
            auto& prim = gMesh["primitives"][pi];
            auto& attrs = prim["attributes"];

            ImportedMeshData mesh;
            mesh.name = meshName + (gMesh["primitives"].size() > 1 ? "_" + std::to_string(pi) : "");

            if (!attrs.contains("POSITION")) continue;

            auto posFloats = getFloats(gltf["accessors"][(unsigned int)attrs["POSITION"]]);

            std::vector<float> normFloats;
            if (attrs.contains("NORMAL"))
                normFloats = getFloats(gltf["accessors"][(unsigned int)attrs["NORMAL"]]);

            std::vector<float> uvFloats;
            if (attrs.contains("TEXCOORD_0"))
                uvFloats = getFloats(gltf["accessors"][(unsigned int)attrs["TEXCOORD_0"]]);

            glm::mat3 normalMatrix = glm::mat3(glm::transpose(glm::inverse(transform)));

            size_t vertCount = posFloats.size() / 3;
            for (size_t i = 0; i < vertCount; i++) {
                Vertex v;
                glm::vec3 localPos(posFloats[i*3], posFloats[i*3+1], posFloats[i*3+2]);
                glm::vec4 globalPos = transform * glm::vec4(localPos, 1.0f);
                v.position = glm::vec3(globalPos);

                glm::vec3 localNorm = (i*3+2 < normFloats.size()) ? glm::vec3(normFloats[i*3], normFloats[i*3+1], normFloats[i*3+2]) : glm::vec3(0,1,0);
                v.normal = glm::normalize(normalMatrix * localNorm);

                v.texUV = (i*2+1 < uvFloats.size()) ? glm::vec2(uvFloats[i*2], 1.0f - uvFloats[i*2+1]) : glm::vec2(0);
                v.color = glm::vec3(1.0f);

                


                mesh.vertices.push_back(v);
            }
            


            if (prim.contains("indices")) {
                mesh.indices = getIndices(gltf["accessors"][(unsigned int)prim["indices"]]);
            } else {
                for (GLuint i = 0; i < (GLuint)vertCount; i++) mesh.indices.push_back(i);
            }

            if (prim.contains("material")) {
                int matIdx = prim["material"];
                if (matIdx < (int)gltf["materials"].size()) {
                    auto& mat = gltf["materials"][matIdx];
                    if (mat.contains("pbrMetallicRoughness")) {
                        auto& pbr = mat["pbrMetallicRoughness"];
                        if (pbr.contains("baseColorFactor")) {
                            auto& bc = pbr["baseColorFactor"];
                            mesh.albedo = glm::vec3(bc[0].get<float>(), bc[1].get<float>(), bc[2].get<float>());
                        }
                        
                        if (pbr.contains("baseColorTexture")) {
                            int texIdx = pbr["baseColorTexture"]["index"];
                            if (gltf.contains("textures") && texIdx < (int)gltf["textures"].size()) {
                                auto& gltfTex = gltf["textures"][texIdx];
                                if (gltfTex.contains("source")) {
                                    int srcIdx = gltfTex["source"];
                                    if (gltf.contains("images") && srcIdx < (int)gltf["images"].size()) {
                                        auto& img = gltf["images"][srcIdx];
                                        if (img.contains("uri")) {
                                            std::string uri = img["uri"];
                                            std::string searchName = fs::path(uri).filename().string();
                                            std::string searchLower = searchName;
                                            std::transform(searchLower.begin(), searchLower.end(), searchLower.begin(), ::tolower);
                                            
                                            std::string modelSubDir = fs::path(filepath).parent_path().string() + "/";
                                            std::vector<std::string> searchDirs = {
                                                modelSubDir,
                                                fs::path(modelSubDir).parent_path().string() + "/",
                                                modelSubDir + "textures/",
                                                modelSubDir + "Textures/",
                                                fs::path(modelSubDir).parent_path().string() + "/textures/",
                                                fs::path(modelSubDir).parent_path().string() + "/Textures/"
                                            };
                                            
                                            std::string diffPath = "";
                                            for (auto& dir : searchDirs) {
                                                if (!fs::exists(dir) || !fs::is_directory(dir)) continue;
                                                for (auto& entry : fs::directory_iterator(dir)) {
                                                    if (!entry.is_regular_file()) continue;
                                                    std::string fnameLower = entry.path().filename().string();
                                                    std::transform(fnameLower.begin(), fnameLower.end(), fnameLower.begin(), ::tolower);
                                                    if (fnameLower == searchLower) {
                                                        diffPath = entry.path().string();
                                                        Logger::AddLog("[ModelImporter] Found glTF texture: %s", diffPath.c_str());
                                                        break;
                                                    }
                                                }
                                                if (!diffPath.empty()) break;
                                            }
                                            
                                            if (!diffPath.empty()) {
                                                mesh.textures.push_back(Texture(diffPath.c_str(), "diffuse"));
                                                mesh.albedo = glm::vec3(1.0f); 
                                            } else {
                                                Logger::AddLog("[ModelImporter] Texture not found: %s", searchName.c_str());
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }

            result.meshes.push_back(std::move(mesh));
        }
    };

    std::function<void(int, glm::mat4)> processNode = [&](int nodeIdx, glm::mat4 parentTransform) {
        if (nodeIdx >= (int)gltf["nodes"].size()) return;
        auto& node = gltf["nodes"][nodeIdx];
        
        glm::mat4 localTransform = glm::mat4(1.0f);
        if (node.contains("matrix")) {
            auto& m = node["matrix"];
            for (int i = 0; i < 16; i++) {
                localTransform[i/4][i%4] = m[i].get<float>();
            }
        } else {
            if (node.contains("translation")) {
                auto& t = node["translation"];
                localTransform = glm::translate(localTransform, glm::vec3(t[0].get<float>(), t[1].get<float>(), t[2].get<float>()));
            }
            if (node.contains("rotation")) {
                auto& r = node["rotation"];
                glm::quat q(r[3].get<float>(), r[0].get<float>(), r[1].get<float>(), r[2].get<float>());
                localTransform *= glm::mat4_cast(q);
            }
            if (node.contains("scale")) {
                auto& s = node["scale"];
                localTransform = glm::scale(localTransform, glm::vec3(s[0].get<float>(), s[1].get<float>(), s[2].get<float>()));
            }
        }
        
        glm::mat4 globalTransform = parentTransform * localTransform;
        
        if (node.contains("mesh")) {
            processMesh(node["mesh"].get<int>(), globalTransform);
        }
        
        if (node.contains("children")) {
            for (auto& childIdx : node["children"]) {
                processNode(childIdx.get<int>(), globalTransform);
            }
        }
    };

    if (gltf.contains("scenes") && gltf.contains("scene")) {
        int defaultScene = gltf["scene"].get<int>();
        auto& scene = gltf["scenes"][defaultScene];
        if (scene.contains("nodes")) {
            for (auto& nodeIdx : scene["nodes"]) {
                processNode(nodeIdx.get<int>(), glm::mat4(1.0f));
            }
        }
    } else if (gltf.contains("nodes")) {
        for (size_t i = 0; i < gltf["nodes"].size(); i++) {
            processNode((int)i, glm::mat4(1.0f));
        }
    } else if (gltf.contains("meshes")) {
        for (size_t i = 0; i < gltf["meshes"].size(); i++) {
            processMesh((int)i, glm::mat4(1.0f));
        }
    }

    
    result.success = !result.meshes.empty();
    if (!result.success) result.error = "No meshes extracted from glTF";
    return result;
}

ImportResult ModelImporter::ImportSTL(const std::string& filepath) {
    ImportResult result;
    result.sourceFile = filepath;

    std::ifstream file(filepath, std::ios::binary);
    if (!file) { result.error = "Cannot open STL file"; return result; }

    char header[80];
    file.read(header, 80);

    std::string headerStr(header, 80);
    bool isAscii = (headerStr.find("solid") == 0);

    file.seekg(0);

    ImportedMeshData mesh;
    mesh.name = fs::path(filepath).stem().string();

    if (!isAscii) {
        file.seekg(80);
        uint32_t numTriangles;
        file.read(reinterpret_cast<char*>(&numTriangles), 4);

        for (uint32_t i = 0; i < numTriangles; i++) {
            float nx, ny, nz;
            file.read(reinterpret_cast<char*>(&nx), 4);
            file.read(reinterpret_cast<char*>(&ny), 4);
            file.read(reinterpret_cast<char*>(&nz), 4);
            glm::vec3 normal(nx, ny, nz);

            for (int v = 0; v < 3; v++) {
                float x, y, z;
                file.read(reinterpret_cast<char*>(&x), 4);
                file.read(reinterpret_cast<char*>(&y), 4);
                file.read(reinterpret_cast<char*>(&z), 4);

                Vertex vertex;
                vertex.position = glm::vec3(x, y, z);
                vertex.normal = normal;
                vertex.color = glm::vec3(1.0f);
                vertex.texUV = glm::vec2(0.0f);

                mesh.indices.push_back(static_cast<GLuint>(mesh.vertices.size()));
                mesh.vertices.push_back(vertex);
            }

            uint16_t attrByteCount;
            file.read(reinterpret_cast<char*>(&attrByteCount), 2);
        }
    } else {
        std::string line;
        glm::vec3 currentNormal(0.0f, 1.0f, 0.0f);

        while (std::getline(file, line)) {
            std::istringstream iss(line);
            std::string token;
            iss >> token;

            if (token == "facet") {
                std::string normalToken;
                iss >> normalToken;
                if (normalToken == "normal") {
                    iss >> currentNormal.x >> currentNormal.y >> currentNormal.z;
                }
            } else if (token == "vertex") {
                Vertex vertex;
                iss >> vertex.position.x >> vertex.position.y >> vertex.position.z;
                vertex.normal = currentNormal;
                vertex.color = glm::vec3(1.0f);
                vertex.texUV = glm::vec2(0.0f);

                mesh.indices.push_back(static_cast<GLuint>(mesh.vertices.size()));
                mesh.vertices.push_back(vertex);
            }
        }
    }

    if (!mesh.vertices.empty()) {
        result.meshes.push_back(std::move(mesh));
        result.success = true;
    } else {
        result.error = "No vertices found in STL file";
    }

    return result;
}




ImportResult ModelImporter::ImportPLY(const std::string& filepath) {
    ImportResult result;
    result.sourceFile = filepath;

    std::ifstream file(filepath);
    if (!file) { result.error = "Cannot open PLY file"; return result; }

    std::string line;
    int vertexCount = 0, faceCount = 0;
    bool inHeader = true;
    bool hasNormals = false;
    bool hasColors = false;

    while (inHeader && std::getline(file, line)) {
        std::istringstream iss(line);
        std::string token;
        iss >> token;

        if (token == "element") {
            std::string elemType;
            iss >> elemType;
            if (elemType == "vertex") iss >> vertexCount;
            else if (elemType == "face") iss >> faceCount;
        } else if (token == "property") {
            std::string propType, propName;
            iss >> propType >> propName;
            if (propName == "nx") hasNormals = true;
            if (propName == "red") hasColors = true;
        } else if (token == "end_header") {
            inHeader = false;
        }
    }

    ImportedMeshData mesh;
    mesh.name = fs::path(filepath).stem().string();

    for (int i = 0; i < vertexCount; i++) {
        if (!std::getline(file, line)) break;
        std::istringstream iss(line);

        Vertex vertex;
        iss >> vertex.position.x >> vertex.position.y >> vertex.position.z;

        if (hasNormals) {
            iss >> vertex.normal.x >> vertex.normal.y >> vertex.normal.z;
        } else {
            vertex.normal = glm::vec3(0.0f, 1.0f, 0.0f);
        }

        if (hasColors) {
            float r, g, b;
            iss >> r >> g >> b;
            vertex.color = glm::vec3(r / 255.0f, g / 255.0f, b / 255.0f);
        } else {
            vertex.color = glm::vec3(1.0f);
        }

        vertex.texUV = glm::vec2(0.0f);
        mesh.vertices.push_back(vertex);
    }

    for (int i = 0; i < faceCount; i++) {
        if (!std::getline(file, line)) break;
        std::istringstream iss(line);

        int numVerts;
        iss >> numVerts;

        std::vector<GLuint> faceIndices(numVerts);
        for (int j = 0; j < numVerts; j++) {
            iss >> faceIndices[j];
        }

        for (int j = 1; j < numVerts - 1; j++) {
            mesh.indices.push_back(faceIndices[0]);
            mesh.indices.push_back(faceIndices[j]);
            mesh.indices.push_back(faceIndices[j + 1]);
        }
    }

    if (!mesh.vertices.empty()) {
        result.meshes.push_back(std::move(mesh));
        result.success = true;
    } else {
        result.error = "No vertices found in PLY file";
    }

    return result;
}
