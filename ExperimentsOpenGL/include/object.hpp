#ifndef OBJECT_HPP
#define OBJECT_HPP

#include "utils.hpp"

#define GL_GLEXT_PROTOTYPES
#include <SDL_opengl.h>

#include <GL/gl.h>
#include <GL/glu.h>
#include <glm/glm.hpp>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <stb_image.h>

#include <vector>
#include <algorithm>
#include <stdexcept>
#include <cstddef>
#include <cassert>

using VecF = std::vector<float>;
using VecUI = std::vector<unsigned int>;

constexpr static int MAX_TEX_PER_KIND = 16;

const char* texture_diffuse_naming_conv[] {
    "texture_diffuse1",
    "texture_diffuse2",
    "texture_diffuse3",
    "texture_diffuse4",
    "texture_diffuse5",
    "texture_diffuse6",
    "texture_diffuse7",
    "texture_diffuse8",
    "texture_diffuse9",
    "texture_diffuse10",
    "texture_diffuse11",
    "texture_diffuse12",
    "texture_diffuse13",
    "texture_diffuse14",
    "texture_diffuse15",
    "texture_diffuse16",
};

const char* texture_specular_naming_conv[] {
    "texture_specular1",
    "texture_specular2",
    "texture_specular3",
    "texture_specular4",
    "texture_specular5",
    "texture_specular6",
    "texture_specular7",
    "texture_specular8",
    "texture_specular9",
    "texture_specular10",
    "texture_specular11",
    "texture_specular12",
    "texture_specular13",
    "texture_specular14",
    "texture_specular15",
    "texture_specular16",
};

const char* texture_normal_naming_conv[] {
    "texture_normal1",
    "texture_normal2",
    "texture_normal3",
    "texture_normal4",
    "texture_normal5",
    "texture_normal6",
    "texture_normal7",
    "texture_normal8",
    "texture_normal9",
    "texture_normal10",
    "texture_normal11",
    "texture_normal12",
    "texture_normal13",
    "texture_normal14",
    "texture_normal15",
    "texture_normal16",
};

const char* texture_height_naming_conv[] {
    "texture_height1",
    "texture_height2",
    "texture_height3",
    "texture_height4",
    "texture_height5",
    "texture_height6",
    "texture_height7",
    "texture_height8",
    "texture_height9",
    "texture_height10",
    "texture_height11",
    "texture_height12",
    "texture_height13",
    "texture_height14",
    "texture_height15",
    "texture_height16",
};

enum class TextureType
{
    normal,
    diffuse,
    specular,
    height
};

class Texture
{
public:
    Texture(const char* filename_, bool alpha = false)
        :filename(filename_)
    {
        std::cout << "loading " << filename_ << std::endl;
        int nrChannels;
        stbi_set_flip_vertically_on_load(true);
        unsigned char* data = stbi_load(filename_, &width, &height, 
            &nrChannels, 0);

        if (data == nullptr) {
            throw std::runtime_error("Cannot load texture.");
        }

        // deduce format
        GLenum format;
        if (nrChannels == 1)
            format = GL_RED;
        else if (nrChannels == 3)
            format = GL_RGB;
        else if (nrChannels == 4)
            format = GL_RGBA;

        glGenTextures(1, &textureID);
        glBindTexture(GL_TEXTURE_2D, textureID);

        gl_texture_settings();

        if (alpha) {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGBA,
                GL_UNSIGNED_BYTE, data);
        } else {
            glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format,
                GL_UNSIGNED_BYTE, data);
            // glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, format,
            //     GL_UNSIGNED_BYTE, data);
        }
        glGenerateMipmap(GL_TEXTURE_2D);

        stbi_image_free(data);
    }

    Texture(const Texture& t)
        : width(t.width)
        , height(t.height)
        , textureID(t.textureID)
        , type(t.type)
        , filename(t.filename)
    { }

    void glBind(int texUnit = 0)
    {
        if (texUnit != 0) {
            glActiveTexture(texUnit);
        }
        glBindTexture(GL_TEXTURE_2D, textureID);
    }

    unsigned int ID() const { return textureID; }

    void setType(TextureType t) { type = t; }
    TextureType getType() const { return type; }
    const std::string& getFilename() const { return filename; }
private:
    int width, height;
    unsigned int textureID;
    TextureType type = TextureType::normal;
    std::string filename;

    void gl_texture_settings()
    {
        // Texture wrapping
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        // Texture filtering
        // for minifying - not needed as we're going to use mipmaps
        // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        // for magnifying
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        // Mipmaps
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, 
            GL_LINEAR_MIPMAP_NEAREST);
    }
};

enum class VertexFormat
{
    // x, y, z
    V,
    // x, y, z, u, v
    VT,
    // x, y, z, r, g, b, u, v
    VCT,
    // x, y, z, n.x, n.y, n.z
    VN,
    // x, y, z, n.x, n.y, n.z, u, v
    VNT
};

class ObjectGen
{
public:
    ObjectGen(VertexFormat f)
        : format(f)
    { }

    ~ObjectGen()
    {
        glDeleteVertexArrays(1, &VAO);
        glDeleteBuffers(1, &VBO);
    }

    void init_vert(const float* array, unsigned int size)
    {
        vertices = VecF(array, array + size);
    }

    void init_ind(const unsigned int* array, unsigned int size)
    {
        indices = VecUI(array, array + size);
    }

    void glPrepare()
    {
        bool ind = !indices.empty();
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        if (ind) {
            glGenBuffers(1, &EBO);
        }
        // bind the Vertex Array Object first, then bind and set vertex 
        // buffer(s), and then configure vertex attributes(s).
        glBindVertexArray(VAO);

        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, vecsizeof(vertices),
            &vertices[0], GL_STATIC_DRAW);

        if (ind) {
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, vecsizeof(indices),
                &indices[0], GL_STATIC_DRAW);
        }
        
        switch (format) {
        case VertexFormat::V:
            // position attribute
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 
                3 * sizeof(float), (void*)0);
            glEnableVertexAttribArray(0);
            trianglesTot = vertices.size() / 3;
            break;
        case VertexFormat::VT:
            // position attribute
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 
                5 * sizeof(float), (void*)0);
            glEnableVertexAttribArray(0);
            // texture coord attribute
            glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 
                5 * sizeof(float), (void*)(3 * sizeof(float)));
            glEnableVertexAttribArray(1);
            trianglesTot = vertices.size() / 5;
            break;
        case VertexFormat::VCT:
        case VertexFormat::VNT:
            // position attribute
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 
                8 * sizeof(float), (void*)0);
            glEnableVertexAttribArray(0);
            // color attribute or normal vector attribute
            glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 
                8 * sizeof(float), (void*)(3 * sizeof(float)));
            glEnableVertexAttribArray(1);
            // texture coord attribute
            glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 
                8 * sizeof(float), (void*)(6 * sizeof(float)));
            glEnableVertexAttribArray(2);
            trianglesTot = vertices.size() / 8;
            break;
        case VertexFormat::VN:
            // position attribute
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 
                6 * sizeof(float), (void*)0);
            glEnableVertexAttribArray(0);
            // normal vectors attribute
            glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 
                6 * sizeof(float), (void*)(3 * sizeof(float)));
            glEnableVertexAttribArray(1);
            trianglesTot = vertices.size() / 6;
            break;
        }

        // note that this is allowed, the call to glVertexAttribPointer
        // registered VBO as the vertex attribute's bound vertex
        // buffer object so afterwards we can safely unbind
        glBindBuffer(GL_ARRAY_BUFFER, 0); 

        // remember: do NOT unbind the EBO while a VAO is active
        // as the bound element buffer object IS stored in the VAO;
        // keep the EBO bound.
        //glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

        // You can unbind the VAO afterwards so other VAO calls
        // won't accidentally modify this VAO, but this rarely happens.
        // Modifying other VAOs requires a call to glBindVertexArray
        // anyways so we generally don't unbind VAOs (nor VBOs)
        // when it's not directly necessary.
        glBindVertexArray(0); 
    }

    void glDraw()
    {
        // seeing as we only have a single VAO there's no need
        // to bind it every time, but we'll do so to keep things
        // a bit more organized
        glBindVertexArray(VAO);
        //glDrawArrays(GL_TRIANGLES, 0, 6);
        if (!indices.empty()) {
            glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
        } else {
            glDrawArrays(GL_TRIANGLES, 0, trianglesTot);
        }
    }

    unsigned int getVBO() const { return VBO; }
private:
    unsigned int VBO, VAO, EBO;
    unsigned int lightVAO;
    unsigned int trianglesTot;
    VertexFormat format;

    VecF vertices;
    VecUI indices;
};

struct Vertex
{
    glm::vec3 pos;
    glm::vec3 normal;
    glm::vec2 tex_coords;
    glm::vec3 tangent;
    glm::vec3 bitangent;
};

class Mesh
{
public:
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    std::vector<Texture> textures;

    Mesh(std::vector<Vertex> verts, std::vector<unsigned int> inds, 
        std::vector<Texture> texs)
        : vertices(verts)
        , indices(inds)
        , textures(texs)
    {
        setupMesh();
    }

    void glDraw(Shader& shader)
    {
        // static bool first = true;
        // bind appropriate textures
        unsigned int diffuseNr  = 0;
        unsigned int specularNr = 0;
        unsigned int normalNr   = 0;
        unsigned int heightNr   = 0;

        for (unsigned int i = 0; i < textures.size(); ++i) {
            // glActiveTexture(GL_TEXTURE0 + i);
            const char* name = nullptr;

            switch (textures[i].getType()) {
            case TextureType::normal:
                assert(normalNr < MAX_TEX_PER_KIND);
                name = texture_normal_naming_conv[normalNr++];
                break;
            case TextureType::diffuse:
                assert(diffuseNr < MAX_TEX_PER_KIND);
                name = texture_diffuse_naming_conv[diffuseNr++];
                break;
            case TextureType::specular:
                assert(specularNr < MAX_TEX_PER_KIND);
                name = texture_specular_naming_conv[specularNr++];
                break;
            case TextureType::height:
                assert(heightNr < MAX_TEX_PER_KIND);
                name = texture_height_naming_conv[heightNr++];
                break;
            }
            assert(name != nullptr);
            shader.setInt(name, i);
            // glBindTexture(GL_TEXTURE_2D, textures[i].ID());
            textures[i].glBind(GL_TEXTURE0 + i);
        }

        // first = false;

        // draw mesh
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);

        // always good practice to set everything back to defaults once configured.
        glActiveTexture(GL_TEXTURE0);
    }
private:
    unsigned int VAO, VBO, EBO;

    void setupMesh()
    {
        // create buffers/arrays
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);

        glBindVertexArray(VAO);
        // load data into vertex buffers
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        // A great thing about structs is that their memory layout is 
        // sequential for all its items.
        // The effect is that we can simply pass a pointer to the struct 
        // and it translates perfectly to a glm::vec3/2 array which
        // again translates to 3/2 floats which translates to a byte array.
        glBufferData(GL_ARRAY_BUFFER, vecsizeof(vertices), 
            &vertices[0], GL_STATIC_DRAW);  

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, vecsizeof(indices), 
            &indices[0], GL_STATIC_DRAW);

        // set the vertex attribute pointers
        // vertex Positions
        glEnableVertexAttribArray(0);	
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 
            (void*)0);
        // vertex normals
        glEnableVertexAttribArray(1);	
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 
            (void*)offsetof(Vertex, normal));
        // vertex texture coords
        glEnableVertexAttribArray(2);	
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), 
            (void*)offsetof(Vertex, tex_coords));
        // vertex tangent
        glEnableVertexAttribArray(3);
        glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 
            (void*)offsetof(Vertex, tangent));
        // vertex bitangent
        glEnableVertexAttribArray(4);
        glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 
            (void*)offsetof(Vertex, bitangent));

        glBindVertexArray(0);
    }
};

class Model 
{
public:
    // stores all the textures loaded so far, 
    // optimization to make sure textures aren't loaded more than once.
    std::vector<Texture> textures_loaded;
    std::vector<Mesh> meshes;
    bool gammaCorrection;
    std::string directory;

    // constructor, expects a filepath to a 3D model.
    Model(const char* path, bool gamma = false)
        : gammaCorrection(gamma)
    {
        loadModel(path);
    }

    // draws the model, and thus all its meshes
    void glDraw(Shader &shader)
    {
        for (auto& m : meshes)
            m.glDraw(shader);
    }
    
private:
    // loads a model with supported ASSIMP extensions 
    // from file and stores the resulting meshes in the meshes vector.
    void loadModel(const char* path)
    {
        Assimp::Importer importer;
        const aiScene* scene = importer.ReadFile(path, 
            aiProcess_Triangulate | aiProcess_GenSmoothNormals 
            | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);
        
        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE 
            || !scene->mRootNode) {
            std::cout << "ERROR::ASSIMP:: " << importer.GetErrorString() 
                << std::endl;
            throw std::runtime_error("Mesh::loadModel failed");
        }

        std::string path_(path);
        directory = path_.substr(0, path_.find_last_of('/'));

        processNode(scene->mRootNode, scene);
    }

    // processes a node in a recursive fashion. 
    // Processes each individual mesh located at the node 
    // and repeats this process on its children nodes (if any).
    void processNode(aiNode *node, const aiScene *scene)
    {
        // process each mesh located at the current node
        for (unsigned int i = 0; i < node->mNumMeshes; ++i) {
            // the node object only contains indices to index 
            // the actual objects in the scene. 
            // the scene contains all the data, node is just to keep 
            // stuff organized (like relations between nodes).
            aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
            meshes.push_back(processMesh(mesh, scene));
        }
        // after we've processed all of the meshes (if any) 
        // we then recursively process each of the children nodes
        for (unsigned int i = 0; i < node->mNumChildren; ++i) {
            processNode(node->mChildren[i], scene);
        }
    }

    Mesh processMesh(aiMesh *mesh, const aiScene *scene)
    {
        std::vector<Vertex> vertices;
        std::vector<unsigned int> indices;
        std::vector<Texture> textures;

        for (unsigned int i = 0; i < mesh->mNumVertices; ++i) {
            Vertex vertex;
            // we declare a placeholder vector since assimp uses its own 
            // vector class that doesn't directly convert to glm's vec3 class 
            // so we transfer the data to this placeholder glm::vec3 first.
            glm::vec3 vector;
            // positions
            vector.x = mesh->mVertices[i].x;
            vector.y = mesh->mVertices[i].y;
            vector.z = mesh->mVertices[i].z;
            vertex.pos = vector;
            // normals
            if (mesh->HasNormals()) {
                vector.x = mesh->mNormals[i].x;
                vector.y = mesh->mNormals[i].y;
                vector.z = mesh->mNormals[i].z;
                vertex.normal = vector;
            }
            // texture coordinates
            // does the mesh contain texture coordinates?
            if (mesh->mTextureCoords[0]) {
                glm::vec2 vec;
                // a vertex can contain up to 8 different texture coordinates.
                // We thus make the assumption that we won't 
                // use models where a vertex can have multiple texture
                // coordinates so we always take the first set (0).
                vec.x = mesh->mTextureCoords[0][i].x; 
                vec.y = mesh->mTextureCoords[0][i].y;
                vertex.tex_coords = vec;
                // tangent
                vector.x = mesh->mTangents[i].x;
                vector.y = mesh->mTangents[i].y;
                vector.z = mesh->mTangents[i].z;
                vertex.tangent = vector;
                // bitangent
                vector.x = mesh->mBitangents[i].x;
                vector.y = mesh->mBitangents[i].y;
                vector.z = mesh->mBitangents[i].z;
                vertex.bitangent = vector;
            } else {
                vertex.tex_coords = glm::vec2(0.0f, 0.0f);
            }

            vertices.push_back(vertex);
        }
        // now wak through each of the mesh's faces (a face is a mesh its triangle)
        // and retrieve the corresponding vertex indices.
        for (unsigned int i = 0; i < mesh->mNumFaces; ++i) {
            aiFace face = mesh->mFaces[i];
            // retrieve all indices of the face and store them in
            // the indices vector
            for (unsigned int j = 0; j < face.mNumIndices; ++j)
                indices.push_back(face.mIndices[j]);        
        }
        // process materials
        aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];    
        // we assume a convention for sampler names in the shaders.
        // Each diffuse texture should be named
        // as 'texture_diffuseN' where N is a sequential number ranging
        // from 1 to MAX_SAMPLER_NUMBER. 
        // Same applies to other texture as the following list summarizes:
        // diffuse: texture_diffuseN
        // specular: texture_specularN
        // normal: texture_normalN

        // 1. diffuse maps
        // std::cout << "loading diffuse" << std::endl;
        std::vector<Texture> diffuseMaps = loadMaterialTextures(material, 
            aiTextureType_DIFFUSE, TextureType::diffuse);
        textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
        // 2. specular maps
        // std::cout << "loading specular" << std::endl;
        std::vector<Texture> specularMaps = loadMaterialTextures(material, 
            aiTextureType_SPECULAR, TextureType::specular);
        textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
        // 3. normal maps
        // std::cout << "loading normal" << std::endl;
        std::vector<Texture> normalMaps = loadMaterialTextures(material, 
            aiTextureType_HEIGHT, TextureType::normal);
        textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());
        // 4. height maps
        // std::cout << "loading height" << std::endl;
        std::vector<Texture> heightMaps = loadMaterialTextures(material, 
            aiTextureType_AMBIENT, TextureType::height);
        textures.insert(textures.end(), heightMaps.begin(), heightMaps.end());
        
        // return a mesh object created from the extracted mesh data
        return Mesh(vertices, indices, textures);
    }

    // checks all material textures of a given type and loads the textures if they're not loaded yet.
    // the required info is returned as a Texture struct.
    std::vector<Texture> loadMaterialTextures(
        aiMaterial *mat, 
        aiTextureType type, 
        TextureType typeIntenal)
    {
        std::vector<Texture> textures;
        for (unsigned int i = 0; i < mat->GetTextureCount(type); ++i) {
            aiString str;
            mat->GetTexture(type, i, &str);
            // check if texture was loaded before and if so,
            // continue to next iteration: skip loading a new texture
            std::string full_path =  directory;
            full_path += '/';
            full_path.append(str.C_Str());
            bool skip = false;
            for (unsigned int j = 0; j < textures_loaded.size(); j++) {
                if (std::strcmp(textures_loaded[j].getFilename().c_str(), 
                    full_path.c_str()) == 0) {
                    textures.push_back(textures_loaded[j]);
                    // a texture with the same filepath has already been loaded,
                    // continue to next one. (optimization)
                    skip = true;
                    break;
                }
            }
            // if texture hasn't been loaded already, load it
            if (!skip) {
                Texture texture(full_path.c_str());
                texture.setType(typeIntenal);
                textures.push_back(texture);
                // store it as texture loaded for entire model,
                // to ensure we won't unnecesery load duplicate textures.
                textures_loaded.push_back(texture); 
            }
        }
        return textures;
    }
};

#endif // OBJECT_HPP