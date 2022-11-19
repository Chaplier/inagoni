VrtxPtr* MyModifier::Initialize(VrtxPtr* inMesh)
{
TMCCountedPtr<I3DShTreeElementMember> member;
TMCCountedPtr<I3DShTreeElement> tree;
TMCCountedPtr<I3DShInstance> instance;
TMCCountedPtr<I3DShExternalPrimitive> fExternalPrimitive;
TMCCountedPtr<I3DExVertexPrimitive> fVertexPrimitive;

TMCCountedPtr<IPolymesh> fPolymesh;
if(TBasicModifier::QueryInterface(IID_I3DShTreeElementMember, (void**)
&member) == MC_S_OK)
{
TMCCountedPtr<I3DShObject> object;
member->GetTreeElement(&tree); ThrowIfNil(tree);
tree->QueryInterface(IID_I3DShInstance, (void**) &instance);
instance->Get3DObject(&object);
object->QueryInterface(IID_I3DShExternalPrimitive, (void**)
&fExternalPrimitive); ThrowIfNil(fExternalPrimitive);

if(fExternalPrimitive)
{
TMCCountedPtr<IShComponent> component;
fExternalPrimitive->GetPrimitiveComponent(&component);
if(component) component->QueryInterface(IID_I3DExVertexPrimitive,
(void**) &fVertexPrimitive);

fVertexPrimitive->GetPolymesh(&fPolymesh,NULL);
if(fVertexPrimitive)
{
TMCCountedPtr<I3DExVertex> fvertex;

// Spin thru vertices and add only LOCKED named vertices to the
// constraint array

for(long index=0; index < fPolymesh->GetNbVertices(); index++)
{
TVector3 fLockedVertex; // holds our vector for assignment to our
array
TMCString255 vrtxname; // holds the name of our vertex

fvertex = fPolymesh->GetVertexNoAddRef(index); // retrieve the
vertex from Carrara
ThrowIfNil(fvertex);
fvertex->GetName(const_cast<char*>(vrtxname.StrGet())); // retrieve
user assigned
// name from Carrara
fvertex->GetPosition(fLockedVertex); // retrieve this vector from
Carrara

if((TMCString255)"Locked" == vrtxname) // is this vertex locked?
inMesh[index].fLocked = true; // this vertex is locked!
else
inMesh[index].fLocked = false; // this vertex is NOT locked, and
free to move

}
}
}
}
return inMesh;
}
