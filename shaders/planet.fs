#version 330 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoord;
in vec3 LocalPos;

uniform vec3 lightPos;
uniform vec3 viewPos;
uniform vec3 objectColor;
uniform bool isSun;
uniform float time;
uniform int planetType; // 0=Sun, 1=Mercury, 2=Venus, 3=Earth, 4=Mars, 5=Jupiter, 6=Saturn, 7=Uranus, 8=Neptune, 9=Moon

// Simple noise function for procedural textures
float hash(vec2 p) {
    return fract(sin(dot(p, vec2(127.1, 311.7))) * 43758.5453123);
}

float noise(vec2 p) {
    vec2 i = floor(p);
    vec2 f = fract(p);
    f = f * f * (3.0 - 2.0 * f);
    
    float a = hash(i);
    float b = hash(i + vec2(1.0, 0.0));
    float c = hash(i + vec2(0.0, 1.0));
    float d = hash(i + vec2(1.0, 1.0));
    
    return mix(mix(a, b, f.x), mix(c, d, f.x), f.y);
}

float fbm(vec2 p) {
    float value = 0.0;
    float amplitude = 0.5;
    float frequency = 1.0;
    
    for(int i = 0; i < 5; i++) {
        value += amplitude * noise(p * frequency);
        frequency *= 2.0;
        amplitude *= 0.5;
    }
    return value;
}

// 3D noise for sphere mapping
float noise3d(vec3 p) {
    vec3 i = floor(p);
    vec3 f = fract(p);
    f = f * f * (3.0 - 2.0 * f);
    
    float n = i.x + i.y * 57.0 + i.z * 113.0;
    return mix(mix(mix(fract(sin(n) * 43758.5453),
                       fract(sin(n + 1.0) * 43758.5453), f.x),
                   mix(fract(sin(n + 57.0) * 43758.5453),
                       fract(sin(n + 58.0) * 43758.5453), f.x), f.y),
               mix(mix(fract(sin(n + 113.0) * 43758.5453),
                       fract(sin(n + 114.0) * 43758.5453), f.x),
                   mix(fract(sin(n + 170.0) * 43758.5453),
                       fract(sin(n + 171.0) * 43758.5453), f.x), f.y), f.z);
}

// 3D FBM using 3D noise
float fbm3d(vec3 p) {
    float value = 0.0;
    float amplitude = 0.5;
    float frequency = 1.0;
    
    for(int i = 0; i < 5; i++) {
        value += amplitude * noise3d(p * frequency);
        frequency *= 2.0;
        amplitude *= 0.5;
    }
    return value;
}

// Turbulence function for more chaotic patterns
float turbulence(vec3 p) {
    float value = 0.0;
    float amplitude = 1.0;
    float frequency = 1.0;
    
    for(int i = 0; i < 6; i++) {
        value += amplitude * abs(noise3d(p * frequency));
        frequency *= 2.0;
        amplitude *= 0.5;
    }
    return value;
}

vec3 getSunColor() {
    // Enhanced sun with vibrant fiery colors - red, orange, and yellow like molten lava
    vec3 p = LocalPos * 3.5;
    
    // Large-scale convection cells (plasma currents)
    float cells = fbm3d(p + vec3(time * 0.08, time * 0.03, 0.0));
    
    // Solar granulation (bubbling surface texture)
    float granulation = noise3d(p * 12.0 + vec3(time * 0.25, time * 0.15, 0.0));
    
    // Active regions with intense magnetic activity
    float activeRegions = turbulence(p * 1.8 + vec3(time * 0.12, time * 0.08, 0.0));
    
    // Solar flares and prominences (bright streaks)
    float flares = pow(turbulence(p * 2.5 + vec3(time * 0.15, time * 0.1, 0.0)), 1.5);
    
    // Sunspots (cooler, darker regions)
    float spots = fbm3d(p * 4.0 + vec3(time * 0.03, 0.0, 0.0));
    float sunspots = smoothstep(0.25, 0.35, spots);
    
    // Chromosphere patterns (dynamic surface features)
    float chromosphere = fbm3d(p * 6.0 + vec3(time * 0.1, 0.0, time * 0.05));
    
    // Volcanic/molten lava veins
    float lavaVeins = turbulence(p * 5.0 + vec3(time * 0.2, 0.0, time * 0.1));
    
    // Hot plasma bursts
    float plasmaBursts = pow(fbm3d(p * 3.5 + vec3(time * 0.18, time * 0.12, 0.0)), 2.0);
    
    // Define FIERY color layers - more red like volcanic lava
    vec3 brightYellow = vec3(1.0, 0.95, 0.7);           // Bright yellow hotspots
    vec3 hotOrange = vec3(1.0, 0.5, 0.15);              // Intense orange fire
    vec3 deepOrange = vec3(1.0, 0.35, 0.08);            // Deep fiery orange
    vec3 volcanoRed = vec3(0.95, 0.25, 0.05);           // Volcanic red-orange
    vec3 lavaRed = vec3(0.9, 0.15, 0.02);               // Molten lava red
    vec3 darkRed = vec3(0.5, 0.08, 0.01);               // Dark red cooler spots
    vec3 brightFlares = vec3(1.0, 0.9, 0.6);            // Brilliant white-yellow flares
    
    // Build the base color with more red emphasis
    vec3 color = mix(volcanoRed, hotOrange, cells * 0.6);
    
    // Add deep lava red regions
    color = mix(color, lavaRed, activeRegions * 0.4);
    
    // Blend in orange-yellow transitional zones
    color = mix(color, deepOrange, chromosphere * 0.35);
    
    // Add molten lava veins (bright red-orange channels)
    float veinPattern = smoothstep(0.6, 0.8, lavaVeins);
    color = mix(color, vec3(1.0, 0.4, 0.1), veinPattern * 0.4);
    
    // Add plasma bursts (intense hot yellow-white regions)
    color = mix(color, brightYellow, plasmaBursts * 0.3);
    
    // Add dark sunspots (cooler dark red regions)
    color = mix(color, darkRed, sunspots * 0.6);
    
    // Add bright solar flares (extreme hotspots)
    color = mix(color, brightFlares, flares * 0.35);
    
    // Add fine granulation detail with red-orange tint
    color += granulation * 0.15 * vec3(1.0, 0.6, 0.3);
    
    // Enhanced limb darkening with deep red tint at edges
    vec3 toView = normalize(viewPos - FragPos);
    float center = dot(normalize(Normal), toView);
    float limb = smoothstep(0.0, 1.0, abs(center));
    
    // Strong limb darkening with deep red-orange at edges
    color *= 0.4 + 0.6 * limb;
    vec3 edgeColor = mix(darkRed * 1.2, color, limb);
    color = mix(edgeColor, color, limb * 0.7);
    
    // Prominent corona glow with red-orange-yellow gradient
    float edge = 1.0 - abs(center);
    float coronaIntensity = pow(edge, 2.2);
    vec3 coronaColor = mix(
        vec3(1.0, 0.3, 0.1),     // Red-orange at outer edge
        vec3(1.0, 0.7, 0.3),     // Orange-yellow middle
        smoothstep(0.0, 1.0, edge)
    );
    color += coronaColor * coronaIntensity * 1.5;
    
    // Add dynamic pulsing bright spots (solar prominences) with red-orange color
    float prominence = sin(time * 2.0 + cells * 10.0) * 0.5 + 0.5;
    color += vec3(1.0, 0.5, 0.2) * prominence * flares * 0.2;
    
    // Add volcanic-style flowing patterns
    float flow = fbm3d(p * 4.0 + vec3(time * 0.15, 0.0, 0.0));
    color = mix(color, vec3(1.0, 0.35, 0.08), flow * 0.15);
    
    // Boost overall intensity
    color *= 2.3;
    
    // Enhance red and orange tones for fiery appearance
    color.r = min(color.r * 1.15, 1.0);
    color.g = min(color.g * 0.85, 1.0);
    color.b *= 0.6;  // Reduce blue for warmer fiery look
    
    return color;
}

vec3 getMercuryColor() {
    // Rocky, heavily cratered surface
    vec3 p = LocalPos * 15.0;
    
    float craters = fbm3d(p);
    float smallCraters = noise3d(p * 3.0);
    float detail = noise3d(p * 8.0);
    
    vec3 baseGray = vec3(0.55, 0.55, 0.58);
    vec3 darkGray = vec3(0.25, 0.25, 0.28);
    vec3 lightGray = vec3(0.7, 0.7, 0.73);
    
    vec3 color = mix(darkGray, baseGray, craters);
    color = mix(color, lightGray, smallCraters * 0.3);
    color *= 1.0 + detail * 0.2;
    
    return color;
}

vec3 getVenusColor() {
    // Thick sulfuric acid clouds with swirling patterns
    vec3 p = LocalPos * 6.0;
    
    float clouds = fbm3d(p + vec3(time * 0.03, 0.0, time * 0.01));
    float swirls = turbulence(p * 0.8 + vec3(time * 0.02, 0.0, 0.0));
    float details = noise3d(p * 12.0);
    
    vec3 cloudBase = vec3(0.95, 0.87, 0.65);
    vec3 cloudDark = vec3(0.75, 0.62, 0.40);
    vec3 cloudBright = vec3(1.0, 0.95, 0.78);
    
    vec3 color = mix(cloudDark, cloudBase, clouds);
    color = mix(color, cloudBright, swirls * 0.25);
    color *= 1.0 + details * 0.1;
    
    return color;
}

vec3 getEarthColor() {
    // Realistic Earth with oceans, continents, clouds, and ice caps
    vec3 p = LocalPos * 5.0;
    
    // Continental shapes (large-scale landmasses)
    float continents = fbm3d(p * 0.8);
    float continentDetail = fbm3d(p * 2.0);
    
    // Ocean waves and depth variation
    float oceanDepth = noise3d(p * 3.0);
    float oceanWaves = noise3d(p * 15.0 + vec3(time * 0.5, 0.0, 0.0)) * 0.03;
    
    // Cloud layers (moving)
    float clouds = fbm3d(p * 3.0 + vec3(time * 0.08, 0.0, time * 0.03));
    float cloudDetail = noise3d(p * 12.0 + vec3(time * 0.1, 0.0, 0.0));
    
    // Polar ice caps based on latitude
    float latitude = abs(LocalPos.y);
    float polarIce = smoothstep(0.7, 0.95, latitude);
    
    // Color definitions
    vec3 deepOcean = vec3(0.01, 0.05, 0.15);
    vec3 shallowOcean = vec3(0.0, 0.3, 0.6);
    vec3 oceanSurface = vec3(0.05, 0.4, 0.7);
    
    vec3 beach = vec3(0.8, 0.75, 0.6);
    vec3 grassland = vec3(0.2, 0.5, 0.15);
    vec3 forest = vec3(0.1, 0.35, 0.1);
    vec3 mountain = vec3(0.5, 0.5, 0.5);
    vec3 desert = vec3(0.7, 0.6, 0.3);
    
    vec3 cloudWhite = vec3(1.0, 1.0, 1.0);
    vec3 iceWhite = vec3(0.9, 0.95, 1.0);
    
    // Determine if location is land or water
    float landThreshold = 0.35;
    float isLand = smoothstep(landThreshold - 0.05, landThreshold + 0.05, continents);
    
    // Ocean color with depth variation
    vec3 oceanColor = mix(deepOcean, shallowOcean, oceanDepth);
    oceanColor = mix(oceanColor, oceanSurface, pow(oceanDepth, 2.0));
    oceanColor += oceanWaves;
    
    // Land color with terrain variation
    vec3 landColor;
    if (continentDetail > 0.6) {
        landColor = mountain; // Mountains
    } else if (continentDetail > 0.45) {
        landColor = mix(grassland, forest, noise3d(p * 5.0)); // Green areas
    } else if (continentDetail > 0.3) {
        landColor = grassland; // Grassland
    } else {
        landColor = desert; // Deserts
    }
    
    // Add beaches at coastlines
    float coastline = abs(isLand - 0.5);
    if (coastline < 0.05 && isLand > 0.4) {
        landColor = mix(landColor, beach, (0.05 - coastline) * 10.0);
    }
    
    // Base color: ocean or land
    vec3 baseColor = mix(oceanColor, landColor, isLand);
    
    // Add polar ice caps
    baseColor = mix(baseColor, iceWhite, polarIce);
    
    // Add clouds (with some transparency)
    float cloudCoverage = smoothstep(0.4, 0.7, clouds) * (1.0 - polarIce);
    cloudCoverage *= 1.0 + cloudDetail * 0.2;
    baseColor = mix(baseColor, cloudWhite, cloudCoverage * 0.6);
    
    return baseColor;
}

vec3 getMarsColor() {
    // Rusty red planet with dust storms and polar caps
    vec3 p = LocalPos * 8.0;
    
    float terrain = fbm3d(p);
    float craters = fbm3d(p * 2.0);
    float dust = turbulence(p * 0.5 + vec3(time * 0.01, 0.0, 0.0));
    
    float latitude = abs(LocalPos.y);
    float polarIce = smoothstep(0.85, 0.95, latitude);
    
    vec3 darkRed = vec3(0.5, 0.2, 0.1);
    vec3 rustRed = vec3(0.8, 0.35, 0.2);
    vec3 lightRed = vec3(0.9, 0.5, 0.3);
    vec3 orangeDust = vec3(0.85, 0.55, 0.35);
    vec3 polarWhite = vec3(0.95, 0.9, 0.85);
    
    vec3 color = mix(darkRed, rustRed, terrain);
    color = mix(color, lightRed, craters * 0.4);
    color = mix(color, orangeDust, dust * 0.3);
    color = mix(color, polarWhite, polarIce);
    
    return color;
}

vec3 getJupiterColor() {
    // Gas giant with prominent bands and Great Red Spot
    vec3 p = LocalPos;
    float y = LocalPos.y;
    
    // Latitude-based banding
    float bands = sin(y * 12.0) * 0.5 + 0.5;
    float bandDetail = fbm3d(vec3(LocalPos.x * 25.0, y * 8.0, LocalPos.z * 25.0));
    
    // Turbulent storm systems
    float storms = turbulence(p * 8.0 + vec3(time * 0.02, 0.0, 0.0));
    
    // Great Red Spot
    vec2 spotCenter = vec2(0.2, -0.15);
    float spotDist = length(LocalPos.xz - spotCenter);
    float redSpot = smoothstep(0.25, 0.12, spotDist);
    float spotSwirl = fbm3d(p * 15.0 + vec3(time * 0.01, 0.0, 0.0));
    
    vec3 lightBand = vec3(0.88, 0.82, 0.68);
    vec3 darkBand = vec3(0.68, 0.52, 0.38);
    vec3 stormColor = vec3(0.78, 0.72, 0.58);
    vec3 redColor = vec3(0.85, 0.38, 0.28);
    
    vec3 color = mix(darkBand, lightBand, bands);
    color = mix(color, stormColor, bandDetail * 0.3);
    color = mix(color, stormColor, storms * 0.15);
    
    // Add Great Red Spot
    vec3 spotColor = mix(redColor, vec3(0.9, 0.5, 0.4), spotSwirl * 0.3);
    color = mix(color, spotColor, redSpot);
    
    return color;
}

vec3 getSaturnColor() {
    // Pale yellow-beige gas giant with subtle bands
    vec3 p = LocalPos;
    float y = LocalPos.y;
    
    float bands = sin(y * 18.0) * 0.5 + 0.5;
    float bandDetail = fbm3d(vec3(LocalPos.x * 20.0, y * 10.0, LocalPos.z * 20.0));
    float haze = noise3d(p * 5.0);
    
    vec3 lightBand = vec3(0.92, 0.88, 0.72);
    vec3 darkBand = vec3(0.82, 0.75, 0.60);
    
    vec3 color = mix(darkBand, lightBand, bands);
    color = mix(color, lightBand * 1.05, bandDetail * 0.2);
    color *= 1.0 + haze * 0.1;
    
    return color;
}

vec3 getUranusColor() {
    // Pale cyan-blue with very subtle features
    vec3 p = LocalPos * 4.0;
    
    float atmosphere = fbm3d(p);
    float clouds = noise3d(p * 2.0 + vec3(time * 0.01, 0.0, 0.0));
    
    vec3 cyan = vec3(0.48, 0.78, 0.82);
    vec3 lightCyan = vec3(0.58, 0.85, 0.88);
    
    vec3 color = mix(cyan, lightCyan, atmosphere * 0.3);
    color *= 1.0 + clouds * 0.08;
    
    return color;
}

vec3 getNeptuneColor() {
    // Deep blue with storm systems
    vec3 p = LocalPos * 6.0;
    
    float atmosphere = fbm3d(p);
    float storms = turbulence(p * 0.8 + vec3(time * 0.015, 0.0, 0.0));
    float bands = sin(LocalPos.y * 10.0) * 0.5 + 0.5;
    
    vec3 deepBlue = vec3(0.12, 0.22, 0.68);
    vec3 brightBlue = vec3(0.22, 0.32, 0.82);
    vec3 stormBlue = vec3(0.30, 0.42, 0.88);
    
    vec3 color = mix(deepBlue, brightBlue, bands * 0.3);
    color = mix(color, brightBlue, atmosphere * 0.3);
    color = mix(color, stormBlue, storms * 0.2);
    
    return color;
}

vec3 getMoonColor() {
    // Gray, cratered lunar surface
    vec3 p = LocalPos * 12.0;
    
    float largeCraters = fbm3d(p);
    float smallCraters = fbm3d(p * 3.0);
    float regolith = noise3d(p * 8.0);
    
    // Mare (dark plains) vs highlands
    float mare = smoothstep(0.35, 0.45, largeCraters);
    
    vec3 highlands = vec3(0.65, 0.65, 0.65);
    vec3 mareColor = vec3(0.35, 0.35, 0.35);
    vec3 craterColor = vec3(0.5, 0.5, 0.5);
    
    vec3 color = mix(mareColor, highlands, mare);
    color = mix(color, craterColor, smallCraters * 0.3);
    color *= 1.0 + regolith * 0.15;
    
    return color;
}

vec3 getPlanetBaseColor() {
    if(planetType == 0) return getSunColor();
    else if(planetType == 1) return getMercuryColor();
    else if(planetType == 2) return getVenusColor();
    else if(planetType == 3) return getEarthColor();
    else if(planetType == 4) return getMarsColor();
    else if(planetType == 5) return getJupiterColor();
    else if(planetType == 6) return getSaturnColor();
    else if(planetType == 7) return getUranusColor();
    else if(planetType == 8) return getNeptuneColor();
    else if(planetType == 9) return getMoonColor();
    else return objectColor;
}

void main()
{
    if (isSun)
    {
        // Sun emits light - bright and glowing
        vec3 sunColor = getSunColor();
        
        // Enhanced pulsing effect with more variation
        float pulse = sin(time * 1.2) * 0.05 + sin(time * 2.5) * 0.03 + 1.0;
        sunColor *= pulse;
        
        // Add bloom-like brightness boost
        sunColor = sunColor * (1.0 + sunColor * 0.3);
        
        FragColor = vec4(sunColor, 1.0);
    }
    else
    {
        // Get procedural planet texture
        vec3 baseColor = getPlanetBaseColor();
        
        // Lighting calculations
        vec3 norm = normalize(Normal);
        vec3 lightDir = normalize(lightPos - FragPos);
        vec3 viewDir = normalize(viewPos - FragPos);
        
        // Ambient
        float ambientStrength = 0.08;
        vec3 ambient = ambientStrength * baseColor;
        
        // Diffuse
        float diff = max(dot(norm, lightDir), 0.0);
        vec3 diffuse = diff * baseColor;
        
        // Specular (different for different planets)
        float specularStrength = 0.0;
        if(planetType == 3) specularStrength = 0.7; // Earth - water reflects strongly
        else if(planetType == 2) specularStrength = 0.4; // Venus - clouds
        else if(planetType == 5 || planetType == 6 || planetType == 7 || planetType == 8) 
            specularStrength = 0.25; // Gas giants - slight shine
        else if(planetType == 9) specularStrength = 0.05; // Moon - very low
        else specularStrength = 0.1; // Rocky planets
        
        vec3 halfwayDir = normalize(lightDir + viewDir);
        float spec = pow(max(dot(norm, halfwayDir), 0.0), 32.0);
        vec3 specular = specularStrength * spec * vec3(1.0);
        
        // Rim lighting (atmospheric glow)
        float rimStrength = 0.0;
        vec3 rimColor = vec3(1.0);
        
        if(planetType == 3) { // Earth
            rimStrength = 0.6;
            rimColor = vec3(0.3, 0.5, 1.0); // Blue atmospheric glow
        } else if(planetType == 2) { // Venus
            rimStrength = 0.4;
            rimColor = vec3(1.0, 0.9, 0.7);
        } else if(planetType == 4) { // Mars
            rimStrength = 0.25;
            rimColor = vec3(1.0, 0.5, 0.3);
        } else if(planetType == 5) { // Jupiter
            rimStrength = 0.3;
            rimColor = vec3(0.9, 0.8, 0.6);
        } else if(planetType == 6) { // Saturn
            rimStrength = 0.3;
            rimColor = vec3(0.95, 0.9, 0.75);
        } else if(planetType == 7 || planetType == 8) { // Uranus, Neptune
            rimStrength = 0.35;
            rimColor = vec3(0.5, 0.7, 1.0);
        }
        
        float rim = 1.0 - max(dot(viewDir, norm), 0.0);
        rim = pow(rim, 3.0);
        vec3 rimLight = rim * rimColor * rimStrength;
        
        // Attenuation based on distance from sun
        float distance = length(lightPos - FragPos);
        float attenuation = 1.0 / (1.0 + 0.0005 * distance + 0.000005 * (distance * distance));
        
        // Combine lighting
        vec3 result = ambient + (diffuse + specular) * attenuation + rimLight;
        
        // Add subtle subsurface scattering for Earth and Mars
        if(planetType == 3 || planetType == 4) {
            float subsurface = max(dot(norm, -lightDir), 0.0);
            result += baseColor * subsurface * 0.12 * attenuation;
        }
        
        FragColor = vec4(result, 1.0);
    }
}
