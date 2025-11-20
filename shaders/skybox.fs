#version 330 core
out vec4 FragColor;

in vec3 TexCoords;

uniform float time;

// Star generation using procedural noise
float hash(vec3 p) {
    p = fract(p * vec3(443.8975, 397.2973, 491.1871));
    p += dot(p, p.yxz + 19.19);
    return fract((p.x + p.y) * p.z);
}

// Improved noise function
float noise3d(vec3 p) {
    vec3 i = floor(p);
    vec3 f = fract(p);
    f = f * f * (3.0 - 2.0 * f);
    
    float n = i.x + i.y * 57.0 + i.z * 113.0;
    return mix(mix(mix(hash(i),
                       hash(i + vec3(1.0, 0.0, 0.0)), f.x),
                   mix(hash(i + vec3(0.0, 1.0, 0.0)),
                       hash(i + vec3(1.0, 1.0, 0.0)), f.x), f.y),
               mix(mix(hash(i + vec3(0.0, 0.0, 1.0)),
                       hash(i + vec3(1.0, 0.0, 1.0)), f.x),
                   mix(hash(i + vec3(0.0, 1.0, 1.0)),
                       hash(i + vec3(1.0, 1.0, 1.0)), f.x), f.y), f.z);
}

// Fractal Brownian Motion
float fbm(vec3 p) {
    float value = 0.0;
    float amplitude = 0.5;
    float frequency = 1.0;
    
    for(int i = 0; i < 4; i++) {
        value += amplitude * noise3d(p * frequency);
        frequency *= 2.0;
        amplitude *= 0.5;
    }
    return value;
}

float stars(vec3 dir) {
    vec3 p = dir * 100.0;
    
    float brightness = 0.0;
    
    // Brighter stars with more visibility
    for(int i = 0; i < 3; i++) {
        vec3 q = floor(p) + 0.5;
        float h = hash(q + float(i) * 100.0);
        
        if(h > 0.992) { // Moderate star density
            vec3 starPos = q + (hash(q + vec3(1.0, 2.0, 3.0) + float(i)) - 0.5) * 0.8;
            float dist = length(p - starPos);
            float size = 0.04 + hash(q + vec3(4.0, 5.0, 6.0)) * 0.08;
            
            // Star brightness with subtle twinkle - BRIGHTER
            float twinkle = 0.8 + 0.2 * sin(time * (2.0 + hash(q + vec3(7.0)) * 2.0) + hash(q + vec3(8.0)) * 6.28);
            float star = smoothstep(size, 0.0, dist) * twinkle;
            
            brightness += star * 1.4; // Increased from 0.9 to 1.4
        }
        
        p *= 2.0;
    }
    
    // Small distant stars - BRIGHTER
    vec3 p2 = dir * 300.0;
    for(int i = 0; i < 2; i++) {
        vec3 q = floor(p2) + 0.5;
        float h = hash(q + float(i) * 200.0);
        
        if(h > 0.985) {
            vec3 starPos = q + (hash(q + vec3(12.0, 13.0, 14.0) + float(i)) - 0.5) * 0.9;
            float dist = length(p2 - starPos);
            float size = 0.02;
            
            float star = smoothstep(size, 0.0, dist);
            brightness += star * 0.7; // Increased from 0.4 to 0.7
        }
        
        p2 *= 1.5;
    }
    
    return brightness;
}

// Distant galaxy ring - provides ambient light
vec3 distantGalaxyRing(vec3 dir) {
    // Create a ring in the distance (like viewing edge-on spiral galaxy)
    float distToPlane = abs(dir.y); // Distance from horizontal plane
    
    // Ring is thin and far away
    float ringWidth = 0.25;
    float ringFalloff = smoothstep(ringWidth, 0.0, distToPlane);
    
    // Add noise for realistic galaxy texture
    float galaxyNoise = fbm(dir * 8.0 + vec3(time * 0.005, 0.0, 0.0));
    float galaxyDetail = fbm(dir * 20.0);
    
    // Ring has brighter and dimmer areas
    float brightness = ringFalloff * (0.5 + galaxyNoise * 0.5);
    brightness *= 0.7 + galaxyDetail * 0.3;
    
    // Distance-based radial gradient (brighter in certain areas)
    float angle = atan(dir.z, dir.x);
    float radialPattern = sin(angle * 3.0 + time * 0.1) * 0.5 + 0.5;
    brightness *= 0.7 + radialPattern * 0.3;
    
    // Galaxy ring colors - MORE WHITE/BRIGHT
    vec3 coreColor = vec3(1.0, 0.95, 0.85);     // Bright white-yellow core
    vec3 armColor = vec3(0.85, 0.88, 0.95);     // Bright blue-white arms
    vec3 dustColor = vec3(0.9, 0.85, 0.95);     // Bright purple-white dust
    
    vec3 galaxyColor = mix(armColor, coreColor, galaxyNoise);
    galaxyColor = mix(galaxyColor, dustColor, galaxyDetail * 0.3);
    
    return galaxyColor * brightness * 0.45; // Subtle glow
}

// Subtle distant nebula glow
vec3 distantNebula(vec3 dir) {
    vec3 p = dir * 3.0;
    
    // Very subtle nebula patches
    float n1 = fbm(p + vec3(time * 0.01, 0.0, 0.0));
    float n2 = fbm(p * 1.8);
    
    // Soft nebula colors
    vec3 nebColor1 = vec3(0.08, 0.04, 0.12) * n1; // Purple
    vec3 nebColor2 = vec3(0.04, 0.08, 0.12) * n2; // Blue
    
    return (nebColor1 + nebColor2) * 0.3;
}

// Milky Way-like band
float milkyWay(vec3 dir) {
    float band = abs(dir.y);
    band = 1.0 - smoothstep(0.0, 0.35, band);
    
    float noise = fbm(dir * 25.0);
    band *= noise * 0.5 + 0.5;
    
    return band * 0.18;
}

void main()
{
    vec3 dir = normalize(TexCoords);
    
    // Base space color - very dark
    vec3 color = vec3(0.002, 0.003, 0.005);
    
    // Add distant galaxy ring for ambient lighting - NOW MORE WHITE AND VISIBLE
    color += distantGalaxyRing(dir);
    
    // REMOVED: Star cluster
    
    // Add very subtle nebula
    color += distantNebula(dir);
    
    // Add subtle Milky Way band
    color += milkyWay(dir) * vec3(0.15, 0.15, 0.18);
    
    // Add brighter stars
    float starBrightness = stars(dir);
    
    // Star color variation - BRIGHTER
    vec3 starColor = vec3(1.0, 0.98, 0.95);
    
    float colorVar = hash(dir * 1000.0);
    if(colorVar > 0.95) {
        starColor = vec3(0.85, 0.92, 1.0); // Brighter blue stars
    } else if(colorVar < 0.05) {
        starColor = vec3(1.0, 0.88, 0.75); // Brighter orange stars
    }
    
    color += starColor * starBrightness;
    
    // Very subtle overall ambient
    color += vec3(0.008, 0.008, 0.01);
    
    // Minimal vignette
    float vignette = length(dir.xy) * 0.5;
    color *= 1.0 - vignette * 0.08;
    
    FragColor = vec4(color, 1.0);
}
