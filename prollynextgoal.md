animation engine // im not qualified enough to do this rn, maybe just a break will do it
i suck at writing shader 


animator is kinda kicking my ass


demo scene for all featuers 

forced optimiation 


fix the hardcoded path  btw 
ray tracing 
path tracing 
nvidia/amd/arc hardware acc 


late game stuff :
create a new UI lib in CPP and not imgui for better bloat free high performance ui



audio optimisation for low latency , small to decent buffer size






debug :

- test with hardcoded object like hardcode the cube to space and see if it gets poerted to build or not  // result : object gets poerted but not visible in standalone build


- SSR is not well implemneted to the build tempelemt casuing it to create invisble objects in standalon build // result : ssr was default off yet having the issue

- check geometry pass and VBO bindings // seems alright

- check material porting because we shifted from regular texture to material formate style rendering


i cant think of anything else lol
