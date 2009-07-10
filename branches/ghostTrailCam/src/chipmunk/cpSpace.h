/* Copyright (c) 2007 Scott Lembcke
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
 
// Number of frames that contact information should persist.
extern int cp_contact_persistence;

// User collision pair function.
typedef int (*cpCollFunc)(cpShape *a, cpShape *b, cpContact *contacts, int numContacts, cpFloat normal_coef, void *data);

// Structure for holding collision pair function information.
// Used internally.
typedef struct cpCollPairFunc {
	unsigned int a;
	unsigned int b;
	cpCollFunc func;
	void *data;
} cpCollPairFunc;

typedef struct cpSpace{
	// Number of iterations to use in the impulse solver.
	int iterations;
//	int sleepTicks;
	
	// Self explanatory.
	cpVect gravity;
	cpFloat damping;
	
	// Time stamp. Is incremented on every call to cpSpaceStep().
	int stamp;

	// The static and active shape spatial hashes.
	cpSpaceHash *staticShapes;
	cpSpaceHash *activeShapes;
	
	// List of bodies in the system.
	cpArray *bodies;
	// List of active arbiters for the impulse solver.
	cpArray *arbiters;
	// Persistant contact set.
	cpHashSet *contactSet;
	
	// List of joints in the system.
	cpArray *joints;
	
	// Set of collisionpair functions.
	cpHashSet *collFuncSet;
	// Default collision pair function.
	cpCollPairFunc defaultPairFunc;
} cpSpace;

// Basic allocation/destruction functions.
cpSpace* cpSpaceAlloc(void);
cpSpace* cpSpaceInit(cpSpace *space);
cpSpace* cpSpaceNew(void);

void cpSpaceDestroy(cpSpace *space);
void cpSpaceFree(cpSpace *space);

// Convenience function. Frees all referenced entities. (bodies, shapes and joints)
void cpSpaceFreeChildren(cpSpace *space);

// Collision pair function management functions.
void cpSpaceAddCollisionPairFunc(cpSpace *space, unsigned int a, unsigned int b,
                                 cpCollFunc func, void *data);
void cpSpaceRemoveCollisionPairFunc(cpSpace *space, unsigned int a, unsigned int b);
void cpSpaceSetDefaultCollisionPairFunc(cpSpace *space, cpCollFunc func, void *data);

// Add and remove entities from the system.
void cpSpaceAddShape(cpSpace *space, cpShape *shape);
void cpSpaceAddStaticShape(cpSpace *space, cpShape *shape);
void cpSpaceAddBody(cpSpace *space, cpBody *body);
void cpSpaceAddJoint(cpSpace *space, cpJoint *joint);

void cpSpaceRemoveShape(cpSpace *space, cpShape *shape);
void cpSpaceRemoveStaticShape(cpSpace *space, cpShape *shape);
void cpSpaceRemoveBody(cpSpace *space, cpBody *body);
void cpSpaceRemoveJoint(cpSpace *space, cpJoint *joint);

// Iterator function for iterating the bodies in a space.
typedef void (*cpSpaceBodyIterator)(cpBody *body, void *data);
void cpSpaceEachBody(cpSpace *space, cpSpaceBodyIterator func, void *data);

// Spatial hash management functions.
void cpSpaceResizeStaticHash(cpSpace *space, cpFloat dim, int count);
void cpSpaceResizeActiveHash(cpSpace *space, cpFloat dim, int count);
void cpSpaceRehashStatic(cpSpace *space);

// Update the space.
void cpSpaceStep(cpSpace *space, cpFloat dt);


static inline int
queryReject(cpShape *a, cpShape *b)
{
	return
		// BBoxes must overlap
	   !cpBBintersects(a->bb, b->bb)
	   // Don't collide shapes attached to the same body.
	   || a->body == b->body
	   // Don't collide objects in the same non-zero group
	   || (a->group && b->group && a->group == b->group)
	   // Don't collide objects that don't share at least on layer.
	   || !(a->layers & b->layers);
}

#include <stdlib.h>

// Callback from the spatial hash.
// TODO: Refactor this into separate functions?
static inline int
queryFunc(void *p1, void *p2, void *data)
{
	// Cast the generic pointers from the spatial hash back to usefull types
	cpShape *a = (cpShape *)p1;
	cpShape *b = (cpShape *)p2;
	cpSpace *space = (cpSpace *)data;
	
	// Reject any of the simple cases
	if(queryReject(a,b)) return 0;
	
	// Shape 'a' should have the lower shape type. (required by cpCollideShapes() )
	if(a->type > b->type){
		cpShape *temp = a;
		a = b;
		b = temp;
	}
	
	// Find the collision pair function for the shapes.
	unsigned int ids[] = {a->collision_type, b->collision_type};
	unsigned int hash = CP_HASH_PAIR(a->collision_type, b->collision_type);
	cpCollPairFunc *pairFunc = (cpCollPairFunc *)cpHashSetFind(space->collFuncSet, hash, ids);
	if(!pairFunc->func) return 0; // A NULL pair function means don't collide at all.
	
	// Narrow-phase collision detection.
	cpContact *contacts = NULL;
	int numContacts = cpCollideShapes(a, b, &contacts);
	if(!numContacts) return 0; // Shapes are not colliding.
	
	// The collision pair function requires objects to be ordered by their collision types.
	cpShape *pair_a = a;
	cpShape *pair_b = b;
	cpFloat normal_coef = 1.0f;
	
	// Swap them if necessary.
	if(pair_a->collision_type != pairFunc->a){
		cpShape *temp = pair_a;
		pair_a = pair_b;
		pair_b = temp;
		normal_coef = -1.0f;
	}
	
	if(pairFunc->func(pair_a, pair_b, contacts, numContacts, normal_coef, pairFunc->data)){
		// The collision pair function OKed the collision. Record the contact information.
		
		// Get an arbiter from space->contactSet for the two shapes.
		// This is where the persistant contact magic comes from.
		cpShape *shape_pair[] = {a, b};
		cpArbiter *arb = (cpArbiter *)cpHashSetInsert(space->contactSet, CP_HASH_PAIR(a, b), shape_pair, space);
		
		// Timestamp the arbiter.
		arb->stamp = space->stamp;
		arb->a = a; arb->b = b; // TODO: Investigate why this is still necessary?
		// Inject the new contact points into the arbiter.
		cpArbiterInject(arb, contacts, numContacts);
		
		// Add the arbiter to the list of active arbiters.
		cpArrayPush(space->arbiters, arb);
		
		return numContacts;
	} else {
		// The collision pair function rejected the collision.
		
		free(contacts);
		return 0;
	}
}

