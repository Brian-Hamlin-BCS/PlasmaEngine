#pragma once
#ifndef LIGHTNING_SYNTAX_TREE_HPP
#define LIGHTNING_SYNTAX_TREE_HPP

namespace Lightning
{
  LightningDeclareStaticLibrary(Syntax, LightningNoNamespace, PlasmaShared);

  namespace EvaluationMode
  {
    enum Enum
    {
      // Parses the entire project (including classes, functions, members, etc)
      Project,

      // Parses just a single input expression
      Expression,
    };
  }

  // This tree stores the parsed language in a format that's easy to traverse
  class PlasmaShared SyntaxTree
  {
  public:

    // Friends
    friend class Parser;

    // Constructor
    SyntaxTree();

    // Destructor
    ~SyntaxTree();

    // Get all the nodes at the given cursor position
    void GetNodesAtCursor(size_t cursorPosition, StringParam cursorOrigin, Array<SyntaxNode*>& nodesOut);

    // Get the graphviz representation for debugging purposes
    String GetGraphVizRepresentation();

    // Show the graphviz representation for debugging purposes
    void ShowGraphVizRepresentation();

  private:
    
    // Recursively walks child nodes looking for any node whose range encompasses the cursor
    void GetNodesAtCursorRecursive(SyntaxNode* node, size_t cursorPosition, StringParam cursorOrigin, Array<SyntaxNode*>& nodesOut);

  public:

    // The root of the tree
    RootNode* Root;

    // A singlular expression to be evaluated (or null if we're compiling an entire tree)
    // Its not actually safe to store the expression here, so we store its parent and index
    // This is also technically not safe, but for now we know we don't do any operations that mess with scopes
    ScopeNode* SingleExpressionScope;
    size_t SingleExpressionIndex;

    // These tokens are generated in response to an 'expect' call that failes in tolerant mode.
    Array<const UserToken*> InvalidTokens;

    // Not copyable
    LightningNoCopy(SyntaxTree);
  };

  #define LightningClonableNode(Type)                   \
    ~Type()                                         \
    {                                               \
      this->DestroyChildren();                      \
    }                                               \
    Type* Clone() const override                    \
    {                                               \
      Type* clone = new Type(*this);                \
                                                    \
      NodeChildren children;                        \
      clone->PopulateChildren(children);            \
      clone->PopulateNonTraversedChildren(children);\
      for (size_t i = 0; i < children.Size(); ++i)  \
      {                                             \
        SyntaxNode*& child = *children[i];          \
        child = child->Clone();                     \
      }                                             \
                                                    \
      SyntaxNode::FixParentPointers(clone, nullptr);\
                                                    \
      return clone;                                 \
    }

  // A syntax node represents any syntactical entity in the syntax tree
  class PlasmaShared SyntaxNode : public ILightningObject
  {
  public:
    LightningDeclareType(SyntaxNode, TypeCopyMode::ReferenceType);

    // Constructor
    SyntaxNode();

    // Destructor
    virtual ~SyntaxNode() {};

    // Copy constructor
    SyntaxNode(const SyntaxNode& toCopy);

    // Convert the node to a string representation
    virtual String ToString() const;

    // Clones a node
    virtual SyntaxNode* Clone() const = 0;

    // Populates an array with the children of this node
    virtual void PopulateChildren(NodeChildren& childrenOut);

    // Populates an array with the non-traversed children of this node
    virtual void PopulateNonTraversedChildren(NodeChildren& childrenOut);

    // Populates an array with SyntaxTypes (we walk all children and single out those that inherit from SyntaxType)
    virtual void PopulateSyntaxTypes(SyntaxTypes& typesOut);

    // Fix all the parent pointers so they point up to their parents
    static void FixParentPointers(SyntaxNode* node, SyntaxNode* parent);

    // Get the merged/trimmed comments for this node
    String GetMergedComments();

    template <typename T>
    T* FindParent()
    {
      if (this->Parent)
        return this->Parent->FindParentOrSelf<T>();
      return nullptr;
    }

    template <typename T>
    T* FindParentOrSelf()
    {
      if (T* result = Type::DynamicCast<T*>(this))
        return result;

      if (this->Parent)
        return this->Parent->FindParentOrSelf<T>();

      return nullptr;
    }

  protected:

    // Destroys all the children (used for cleanup)
    void DestroyChildren();

  private:

    // Directly add a child (regardless of lock mode)
    void DirectAdd(SyntaxNode* node);

    // Directly remove a child (regardless of lock mode)
    void DirectRemove(SyntaxNode* node);

  public:

    // Store the parent pointer
    SyntaxNode* Parent;

    // The location that the syntax node originated from
    CodeLocation Location;

    // Any comments collected for this syntax node (used for documentation / translation)
    StringArray Comments;

    // In cases where we generate a temporary (such as creation call nodes for initializer lists, indexers, etc)
    // we will create a local variable node as an expression that need not be walked by formatters and translators
    bool IsGenerated;
  };

  // A pre-type representation for the syntax tree
  class PlasmaShared SyntaxType : public SyntaxNode
  {
  public:
    LightningDeclareType(SyntaxType, TypeCopyMode::ReferenceType);

    // Constructor
    SyntaxType();

    // Tells us if a particular declarations of a syntax type is a template instantiation
    virtual bool IsTemplateInstantiation() const;

    // Whatever type this syntax type resovled to
    Type* ResolvedType;
  };

  class PlasmaShared AnySyntaxType : public SyntaxType
  {
  public:
    LightningDeclareType(AnySyntaxType, TypeCopyMode::ReferenceType);
    LightningClonableNode(AnySyntaxType);

    // SyntaxType interface
    String ToString() const override;
  };

  class PlasmaShared IndirectionSyntaxType : public SyntaxType
  {
  public:
    LightningDeclareType(IndirectionSyntaxType, TypeCopyMode::ReferenceType);
    LightningClonableNode(IndirectionSyntaxType);

    // Constructor
    IndirectionSyntaxType();

    // The syntax type that we refer to
    SyntaxType* ReferencedType;

    // SyntaxType interface
    String ToString() const override;
    virtual void PopulateChildren(NodeChildren& childrenOut) override;
  };

  class PlasmaShared BoundSyntaxType : public SyntaxType
  {
  public:
    LightningDeclareType(BoundSyntaxType, TypeCopyMode::ReferenceType);
    LightningClonableNode(BoundSyntaxType);

    // Store the name of the type
    String TypeName;

    // Template arguments (if we have any)
    // These can be constant ExpressionNodes (not guaranteed to be constant yet) or SyntaxTypes
    NodeList<SyntaxNode> TemplateArguments;
    
    // SyntaxType interface
    bool IsTemplateInstantiation() const override;
    String ToString() const override;
    virtual void PopulateChildren(NodeChildren& childrenOut) override;
  };

  // A parameter that belongs inside of a delegate declaration
  class PlasmaShared DelegateSyntaxParameter
  {
  public:
    // Constructor
    DelegateSyntaxParameter();

    // A parameter generally has a name
    const UserToken* Name;

    // A parameter also has a type
    SyntaxType* Type;
  };

  class PlasmaShared DelegateSyntaxType : public SyntaxType
  {
  public:
    LightningDeclareType(DelegateSyntaxType, TypeCopyMode::ReferenceType);
    LightningClonableNode(DelegateSyntaxType);

    // Constructor
    DelegateSyntaxType();

    // Template arguments (if we have any)
    SyntaxTypeList TemplateArguments;

    // Store the variable types as well as their names
    Array<DelegateSyntaxParameter> Parameters;

    // The return type of the delegate (or null for no return type)
    SyntaxType* Return;
    
    // SyntaxType interface
    bool IsTemplateInstantiation() const override;
    String ToString() const override;
    virtual void PopulateChildren(NodeChildren& childrenOut) override;
  };

  // Whether or not we're virtual or overriding
  namespace VirtualMode
  {
    enum Enum
    {
      NonVirtual,
      Virtual,
      Overriding
    };
  }

  // A root node is the root of a syntax tree
  class PlasmaShared RootNode : public SyntaxNode
  {
  public:
    LightningDeclareType(RootNode, TypeCopyMode::ReferenceType);
    LightningClonableNode(RootNode);

    // SyntaxNode interface
    void PopulateChildren(NodeChildren& childrenOut) override;

    // Store all the root level classes
    NodeList<ClassNode> Classes;

    // Store all the root level enums
    NodeList<EnumNode> Enums;

    // Contains all classes, enums, etc in the order they are declared
    NodeList<SyntaxNode> NonTraversedNonOwnedNodesInOrder;
  };

  // An attribute that can be attached to classes, functions, member variables, etc
  class PlasmaShared AttributeNode : public SyntaxNode
  {
  public:
    LightningDeclareType(AttributeNode, TypeCopyMode::ReferenceType);
    LightningClonableNode(AttributeNode);

    // Default constructor
    AttributeNode();

    // SyntaxNode interface
    void PopulateChildren(NodeChildren& childrenOut) override;

    // The attribute type name
    const UserToken* TypeName;

    // An optional node for when the user wants to pass parameters to an attribute
    FunctionCallNode* AttributeCall;
  };

  // A statement node represents any kind of statement
  class PlasmaShared StatementNode : public SyntaxNode
  {
  public:
    LightningDeclareType(StatementNode, TypeCopyMode::ReferenceType);

    // Returns if the given node is used as a statement
    // For example: Expressions are statements, but are only considered
    // being used as a statement when they appear alone, eg 'i += 5;'
    static bool IsNodeUsedAsStatement(SyntaxNode* node);

    // Clones a node
    virtual StatementNode* Clone() const = 0;
  };

  // An evaluatable node represents anything that can be evaluated into a value (expressions, function calls, values, etc)
  class PlasmaShared ExpressionNode : public StatementNode
  {
  public:
    LightningDeclareType(ExpressionNode, TypeCopyMode::ReferenceType);

    // Constructor
    ExpressionNode();

    // Clones a node
    virtual ExpressionNode* Clone() const = 0;

    // Store the type along with the expression (this will be filled in later)
    Type* ResultType;

    // This type is filled out when we want to know an expression's result type BEFORE we run all the expression type
    // checking, otherwise it will be null. This is used when inferring a type of a member variable
    Type* PrecomputedResultType;

    // Stores how we access this particular expression (stack, member, etc)
    Operand Access;

    // How people are allowed to use this value
    IoMode::Enum Io;

    // How it is trying to be used by it's parent node
    // If this value conflicts with the node's IO mode, then it will result in an error
    // This value is also used to determine whether we call the get/set or both for properties
    IoMode::Enum IoUsage;

    // This determines whether or not this node is being used as a statement
    // See 'IsNodeUsedAsStatement' on StatementNode
    bool IsUsedAsStatement;
  };

  // A binary-operator node represents a binary operator and its operands
  class PlasmaShared BinaryOperatorNode : public ExpressionNode
  {
  public:
    LightningDeclareType(BinaryOperatorNode, TypeCopyMode::ReferenceType);
    LightningClonableNode(BinaryOperatorNode);

    // Constructor
    BinaryOperatorNode();
    
    // SyntaxNode interface
    String ToString() const override;
    void PopulateChildren(NodeChildren& childrenOut) override;

    // The left and right arguments that the binary operator is being applied to
    ExpressionNode* LeftOperand;
    ExpressionNode* RightOperand;

    // All the info we need about the operator (filled out by the syntaxer)
    BinaryOperator OperatorInfo;

    // The operator that tells us what kind of binary operation this is
    const UserToken* Operator;
  };

  // A unary-operator node represents a unary operator and its operand
  class PlasmaShared UnaryOperatorNode : public ExpressionNode
  {
  public:
    LightningDeclareType(UnaryOperatorNode, TypeCopyMode::ReferenceType);
    LightningClonableNode(UnaryOperatorNode);

    // Constructor
    UnaryOperatorNode();
    
    // SyntaxNode interface
    String ToString() const override;
    void PopulateChildren(NodeChildren& childrenOut) override;

    // The single argument of the unary operator
    ExpressionNode* Operand;

    // All the info we need about the operator
    UnaryOperator OperatorInfo;

    // The operator that tells us what kind of unary operation this is
    const UserToken* Operator;
  };

  // A unary-operator node represents a unary operator and its operand
  class PlasmaShared PropertyDelegateOperatorNode : public UnaryOperatorNode
  {
  public:
    LightningDeclareType(PropertyDelegateOperatorNode, TypeCopyMode::ReferenceType);
    LightningClonableNode(PropertyDelegateOperatorNode);

    // Constructor
    PropertyDelegateOperatorNode();

    // The property we're associated with
    Property* AccessedProperty;
  };

  // A type-cast node represents a type cast from an expression to a specified type
  class PlasmaShared TypeCastNode : public ExpressionNode
  {
  public:
    LightningDeclareType(TypeCastNode, TypeCopyMode::ReferenceType);
    LightningClonableNode(TypeCastNode);

    // Constructor
    TypeCastNode();
    
    // SyntaxNode interface
    String ToString() const override;
    void PopulateChildren(NodeChildren& childrenOut) override;

    // The type of cast operation we do
    CastOperator OperatorInfo;

    // Store the expression that will be casted
    ExpressionNode* Operand;

    // Name of the type that we represent
    SyntaxType* Type;
  };


  // A post expression node represents right hand operators (call, indexer, access, etc)
  class PlasmaShared PostExpressionNode : public ExpressionNode
  {
  public:
    LightningDeclareType(PostExpressionNode, TypeCopyMode::ReferenceType);

    // Constructor
    PostExpressionNode();
    
    // SyntaxNode interface
    void PopulateChildren(NodeChildren& childrenOut) override;

    // Post expressions are special expressions that come after an operand
    ExpressionNode* LeftOperand;
  };

  // An indexer call node represents a list of passed in arguments used in an indexer call
  class PlasmaShared IndexerCallNode : public PostExpressionNode
  {
  public:
    LightningDeclareType(IndexerCallNode, TypeCopyMode::ReferenceType);
    LightningClonableNode(IndexerCallNode);
    
    // Constructor
    IndexerCallNode();

    // SyntaxNode interface
    String ToString() const override;
    void PopulateChildren(NodeChildren& childrenOut) override;

    // The list of arguments, provided in order, to the indexer
    NodeList<ExpressionNode> Arguments;

    // The indexer can end up invoking just the Get, or Get and then Set, or just Set
    // The parser generates all three possibilities, and the Syntaxer chooses the correct
    // one based on the usage (compound operators such as += will chosee the GetSet version, but = will just choose Set)
    // The above arguments actually get cloned into each of these possibilities
    // In the future, we may try and reduce this so not all of these have to be generated (its a lot of extra data)
    MultiExpressionNode* Get;
    MultiExpressionNode* GetSet;
    MultiExpressionNode* Set;
  };

  // A function call node represents a list of passed in arguments used in a function call
  class PlasmaShared FunctionCallNode : public PostExpressionNode
  {
  public:
    LightningDeclareType(FunctionCallNode, TypeCopyMode::ReferenceType);
    LightningClonableNode(FunctionCallNode);

    // Constructor
    FunctionCallNode();
    
    // SyntaxNode interface
    String ToString() const override;
    void PopulateChildren(NodeChildren& childrenOut) override;

    // If the left operand is a local variable whose initializer is a creation call node, this will return that node
    StaticTypeNode* FindCreationCall();

    // An array of all the names given to the arguments
    // Empty if we're doing a standard call
    StringArray ArgumentNames;

    // Store the actual expressions passed in for each argument
    NodeList<ExpressionNode> Arguments;

    // Maps the arguments in their passed in order to the actual argument order of the function
    Array<size_t> ArgumentMap;

    // If the call is done in named style, then we have no argument names
    bool IsNamed;
  };

  namespace MemberAccessType
  {
    enum Enum
    {
      Invalid,
      Field,
      Property,
      Function,
      Dynamic
    };
  }

  // A member-access node represents accessing a member / field
  class PlasmaShared MemberAccessNode : public PostExpressionNode
  {
  public:
    LightningDeclareType(MemberAccessNode, TypeCopyMode::ReferenceType);
    LightningClonableNode(MemberAccessNode);

    // Constructor
    MemberAccessNode();
    
    // SyntaxNode interface
    String ToString() const override;

    // The name that we're accessing
    String Name;

    // If this is a static access
    bool IsStatic;

    // The operator used to access (eg '.')
    Grammar::Enum Operator;

    // The type of member we're accessing
    MemberAccessType::Enum MemberType;

    // This is always set to the member that is being accessed
    Member* AccessedMember;

    // If the member is a property (getter setter or field) this will be set
    Property* AccessedProperty;
    
    // If this node is a getter setter access node, then this refers to which property
    GetterSetter* AccessedGetterSetter;

    // If this node is a field access node, then this refers to which field
    Field* AccessedField;

    // This is needed since a function can actually be overloaded
    const FunctionArray* OverloadedFunctions;
    Function* AccessedFunction;
  };

  // Lets us get the runtime type object that describes a type
  class PlasmaShared TypeIdNode : public ExpressionNode
  {
  public:
    LightningDeclareType(TypeIdNode, TypeCopyMode::ReferenceType);
    LightningClonableNode(TypeIdNode);

    // Constructor
    TypeIdNode();

    // SyntaxNode interface
    void PopulateChildren(NodeChildren& childrenOut) override;

    // The syntax type that we need runtime type identification for
    // This may be null if the 'value' expression node is set
    SyntaxType* CompileTimeSyntaxType;

    // The expression we need to get the type of
    // This may be null if a sytax type is set!
    ExpressionNode* Value;

    // The type we resolved for the expression or static type given to
    // typeid at compile time. In the case of handles and delegates,
    // the type will be resolved further in opcode
    Type* CompileTimeType;
  };

  // Lets us get the runtime property object that describes a field or getter/setter
  class PlasmaShared MemberIdNode : public ExpressionNode
  {
  public:
    LightningDeclareType(MemberIdNode, TypeCopyMode::ReferenceType);
    LightningClonableNode(MemberIdNode);

    // Constructor
    MemberIdNode();

    // SyntaxNode interface
    void PopulateChildren(NodeChildren& childrenOut) override;

    // The member that we are attempting to access
    MemberAccessNode* Member;
  };

  namespace CreationMode
  {
    enum Enum
    {
      Invalid,
      New,
      Local
    };
  }

  // Used when we access static members as well as invoking constructors (could be after new/local, or just by itself)
  class PlasmaShared StaticTypeNode : public ExpressionNode
  {
  public:
    LightningDeclareType(StaticTypeNode,TypeCopyMode::ReferenceType);
    LightningClonableNode(StaticTypeNode);

    // Constructor
    StaticTypeNode();

    // SyntaxNode interface
    void PopulateChildren(NodeChildren& childrenOut) override;
    
    // The syntax type that we are accessing
    BoundSyntaxType* ReferencedSyntaxType;

    // The type we resolve to what we're accessing
    BoundType* ReferencedType;
    
    // The below members are ONLY used if this node is being used in a constructor call:
    // The token we used to create this (new, local, etc)
    CreationMode::Enum Mode;

    // The constructor we're running, or null for pre-constructor only
    const FunctionArray* OverloadedConstructors;
    Function* ConstructorFunction;

    // We always create a handle to the type; for example, new always returns a handle, and we
    // always need a handle for preconstructor and constructor calls, even on local objects
    OperandIndex ThisHandleLocal;
  };

  // When we want to initialize a type we can also initialize particular members
  class PlasmaShared ExpressionInitializerMemberNode : public SyntaxNode
  {
  public:
    LightningDeclareType(ExpressionInitializerMemberNode,TypeCopyMode::ReferenceType);
    LightningClonableNode(ExpressionInitializerMemberNode);

    // Constructor
    ExpressionInitializerMemberNode();
    
    // SyntaxNode interface
    void PopulateChildren(NodeChildren& childrenOut) override;

    // The name of the member that we're initializing
    UserToken MemberName;

    // The value that we want to initialize the member to
    ExpressionNode* Value;
  };

  // When we want to initialize a type we can also add values to it (generally for containers)
  class PlasmaShared ExpressionInitializerAddNode : public SyntaxNode
  {
  public:
    LightningDeclareType(ExpressionInitializerAddNode, TypeCopyMode::ReferenceType);
    LightningClonableNode(ExpressionInitializerAddNode);
    
    // SyntaxNode interface
    void PopulateChildren(NodeChildren& childrenOut) override;

    // These arguments get directly passed in to a call to add on the given container
    NodeList<ExpressionNode> Arguments;
  };
  
  // When we want to initialize a type (either a container, with .Add calls, or members of a class / properties)
  class PlasmaShared ExpressionInitializerNode : public PostExpressionNode
  {
  public:
    LightningDeclareType(ExpressionInitializerNode, TypeCopyMode::ReferenceType);
    LightningClonableNode(ExpressionInitializerNode);

    // Constructor
    ExpressionInitializerNode();
    
    // SyntaxNode interface
    void PopulateChildren(NodeChildren& childrenOut) override;

    // All the elements we want to add to the container (by literally invoking .Add)
    NodeList<ExpressionInitializerAddNode> AddValues;

    // All the members we want to initialize
    NodeList<ExpressionInitializerMemberNode> InitailizeMembers;

    // The above element expressions get translated directly into statements
    // This is primarily used for code generation (the above is just syntactic sugar)
    // For example, for the 'add values' to a container, it gets translated into object.Add(value, value...)
    // Member initializers get translated into object.MemberName = value;
    // Warning: many of these nodes point unsafely at another node above in the tree (eg at the creation call itself)
    NodeList<ExpressionNode> InitializerStatements;
  };
  
  // A multi-expression Contains multiple expressions but only yields the results of one of them (done by index)
  class PlasmaShared MultiExpressionNode : public ExpressionNode
  {
  public:
    LightningDeclareType(MultiExpressionNode, TypeCopyMode::ReferenceType);
    LightningClonableNode(MultiExpressionNode);

    // Constructor
    MultiExpressionNode();
    
    // SyntaxNode interface
    void PopulateChildren(NodeChildren& childrenOut) override;

    // All the expressions that the multi-expression will execute
    NodeList<ExpressionNode> Expressions;

    // An index into the array of expressions that controls what this expression results in
    // Basically we just forward our ResultType and Access to that node
    // It is an error to leave this index unset
    size_t YieldChildExpressionIndex;

    // Initialize the yield index to this (we will internal error in the Syntaxer if this is still set)
    static const size_t InvalidIndex = (size_t)-1;
  };

  // A variable node represents any variable declaration
  class PlasmaShared VariableNode : public ExpressionNode
  {
  public:
    LightningDeclareType(VariableNode, TypeCopyMode::ReferenceType);

    // Constructor
    VariableNode();
    
    // SyntaxNode interface
    String ToString() const override;
    void PopulateChildren(NodeChildren& childrenOut) override;

    // Check if the node is inferred (denoted by ResultSyntaxType being null)
    bool IsInferred() const;

    // Store the variable name
    UserToken Name;

    // The initial value assigned to the variable
    ExpressionNode* InitialValue;

    // Is the variable static?
    bool IsStatic;

    // Name of the type that we represent
    SyntaxType* ResultSyntaxType;
  };

  // A local variable node represents a local variable declaration (such as one inside a function)
  class PlasmaShared LocalVariableNode : public VariableNode
  {
  public:
    LightningDeclareType(LocalVariableNode, TypeCopyMode::ReferenceType);
    LightningClonableNode(LocalVariableNode);

    // Constructor
    LocalVariableNode();

    // Constructor to generate a unique local variable node
    // This case is generally for when we want to wrap a local stack value with a name we can lookup later
    // We're typically using the local variable as an expression that wraps the initial value
    LocalVariableNode(StringParam baseName, Project* parentProject, ExpressionNode* optionalInitialValue);

    // Store a pointer that gives information about the local variable
    Variable* CreatedVariable;

    // If this is set, it means this local variable will not generate temporary space, but instead
    // will directly forward access to its initial value expression (requires the initial value to exist!)
    bool ForwardLocalAccessIfPossible;

    // A pointer to the attributes this function has
    NodeList<AttributeNode> Attributes;
  };

  // A parameter node represents the parameter of a function
  class PlasmaShared ParameterNode : public LocalVariableNode
  {
  public:
    LightningDeclareType(ParameterNode, TypeCopyMode::ReferenceType);
    LightningClonableNode(ParameterNode);

    // Constructor
    ParameterNode();

    // Which parameter this is in the function
    size_t ParameterIndex;
  };

  // A member variable node represents a member variable declaration (such as one inside a class)
  class PlasmaShared MemberVariableNode : public VariableNode
  {
  public:
    LightningDeclareType(MemberVariableNode, TypeCopyMode::ReferenceType);
    LightningClonableNode(MemberVariableNode);
    
    // Constructor
    MemberVariableNode();

    // SyntaxNode interface
    void PopulateChildren(NodeChildren& childrenOut) override;

    // Store the associated member on this node
    Field* CreatedField;
    GetterSetter* CreatedGetterSetter;
    Property* CreatedProperty;

    // Store the parent class type
    BoundType* ParentClassType;

    // Store the resulting type of the node
    Type* ResultType;

    // The get and set functions for this variable
    // Only valid for properties, we always know that either the get or set will exist (or both)
    FunctionNode* Get;
    FunctionNode* Set;

    // Whether or not this is a property
    // Note that this also tells us if the get/set are generated
    bool IsGetterSetter;

    // A pointer to the attributes this function has
    NodeList<AttributeNode> Attributes;

    // If this is an extension method, this will be the new owner of the method
    BoundSyntaxType* ExtensionOwner;

    // Is the function a virtual function (or overriding)?
    VirtualMode::Enum Virtualized;
  };

  // A value node represents any constant or identifier value
  class PlasmaShared ValueNode : public ExpressionNode
  {
  public:
    LightningDeclareType(ValueNode, TypeCopyMode::ReferenceType);
    LightningClonableNode(ValueNode);

    // Constructor
    ValueNode();

    // Returns the type of the value node based entirely on the token (or null if it's invalid)
    Type* PrecomputeType() const;

    // SyntaxNode interface
    String ToString() const override;

    // Store the token that represents the value
    UserToken Value;
  };

  // String interpolants are basically advanced efficient string concatenations with values
  class PlasmaShared StringInterpolantNode : public ExpressionNode
  {
  public:
    LightningDeclareType(StringInterpolantNode, TypeCopyMode::ReferenceType);
    LightningClonableNode(StringInterpolantNode);

    // Constructor
    StringInterpolantNode();
    
    // SyntaxNode interface
    void PopulateChildren(NodeChildren& childrenOut) override;

    // All the elements of the interpolant, in order of concatenation
    // The elements will be converted into string types during the interpolation
    NodeList<ExpressionNode> Elements;
  };

  // A delete node represents explicit deletion of an object
  class PlasmaShared DeleteNode : public StatementNode
  {
  public:
    LightningDeclareType(DeleteNode, TypeCopyMode::ReferenceType);
    LightningClonableNode(DeleteNode);

    // Constructor
    DeleteNode();
    
    // SyntaxNode interface
    void PopulateChildren(NodeChildren& childrenOut) override;

    // The object that we'd like to delete
    ExpressionNode* DeletedObject;
  };

  // An return node represents the return statement for a function
  class PlasmaShared ReturnNode : public StatementNode
  {
  public:
    LightningDeclareType(ReturnNode, TypeCopyMode::ReferenceType);
    LightningClonableNode(ReturnNode);

    // Constructor
    ReturnNode();
    
    // SyntaxNode interface
    void PopulateChildren(NodeChildren& childrenOut) override;

    // Store the expression that is to be returned by this statement
    ExpressionNode* ReturnValue;

    // If this is a debug return, then we will ignore flow control errors
    bool IsDebugReturn;
  };

  // A scope node represends a type of scope
  class PlasmaShared ScopeNode : public StatementNode
  {
  public:
    LightningDeclareType(ScopeNode, TypeCopyMode::ReferenceType);
    LightningClonableNode(ScopeNode);

    // Constructor
    ScopeNode();
    
    // SyntaxNode interface
    void PopulateChildren(NodeChildren& childrenOut) override;

    // The statements executed if the condition is met
    NodeList<StatementNode> Statements;

    // Tells us if this is a closed path
    // Note that if the processed statements mark this node
    // as being a full return, then the scope node itself
    // needs to report to its parent scope that it is a full return
    bool AllPathsReturn;
    bool IsDebugReturn;

    // Any variables that belong to this scope
    VariableMap ScopedVariables;
  };

  // Allows code to run for a period of time before it throws an exception and 'times out'
  class PlasmaShared TimeoutNode : public ScopeNode
  {
  public:
    LightningDeclareType(TimeoutNode, TypeCopyMode::ReferenceType);
    LightningClonableNode(TimeoutNode);

    // Constructor
    TimeoutNode();

    // The number of seconds that the timeout will last for
    size_t Seconds;
  };

  // An if node represents the if-then else-if else construct
  class PlasmaShared IfNode : public ScopeNode
  {
  public:
    LightningDeclareType(IfNode, TypeCopyMode::ReferenceType);
    LightningClonableNode(IfNode);

    // Constructor
    IfNode();
    
    // SyntaxNode interface
    void PopulateChildren(NodeChildren& childrenOut) override;

    // Marks whether this is the first part of the if statement (not an else if or else)
    bool IsFirstPart;

    // The conditional expression used in this if statement
    // Non null for all if elses, and only the last CAN be null, but may not be null!
    ExpressionNode* Condition;
  };

  // We hold all parts of the if as children
  class PlasmaShared IfRootNode : public StatementNode
  {
  public:
    LightningDeclareType(IfRootNode, TypeCopyMode::ReferenceType);
    LightningClonableNode(IfRootNode);

    // Constructor
    IfRootNode();
    
    // SyntaxNode interface
    void PopulateChildren(NodeChildren& childrenOut) override;

    // All parts of the if statement
    // The first node in this list is the if itself (not an else!)
    // All nodes after that are 'else if' nodes and have conditions, except
    // the last one can omit the condition and be just an 'else' node
    NodeList<IfNode> IfParts;
  };

  // Declares that a class sends a particular type of event
  class PlasmaShared SendsEventNode : public SyntaxNode
  {
  public:
    LightningDeclareType(SendsEventNode, TypeCopyMode::ReferenceType);
    LightningClonableNode(SendsEventNode);

    // Constructor
    SendsEventNode();

    // SyntaxNode interface
    void PopulateChildren(NodeChildren& childrenOut) override;

    // The type of event we send
    BoundSyntaxType* EventType;

    // The name of the event we send
    const UserToken* Name;

    // The static property that allows users access to the event (string type)
    Property* EventProperty;

    // A pointer to the attributes this sends has
    NodeList<AttributeNode> Attributes;
  };

  // An break node represents the break statement in a loop
  class PlasmaShared BreakNode : public StatementNode
  {
  public:
    LightningDeclareType(BreakNode, TypeCopyMode::ReferenceType);
    LightningClonableNode(BreakNode);

    // Constructor
    BreakNode();

    // How many scopes we wish to break out of (default 1)
    size_t ScopeCount;

    // The instruction index for where the jump occurs
    size_t InstructionIndex;

    // The jump opcode that's associated with our continue (where we jump to)
    // We need to store this so that, after we build code for a function, we can
    // come back to this statement and setup the jump to the end of the loop
    RelativeJumpOpcode* JumpOpcode;
  };

  // An break node represents a debug breakpoint in the virtual machine
  class PlasmaShared DebugBreakNode : public StatementNode
  {
  public:
    LightningDeclareType(DebugBreakNode, TypeCopyMode::ReferenceType);
    LightningClonableNode(DebugBreakNode);
  };

  // An continue node represents the continue statement in a loop
  class PlasmaShared ContinueNode : public StatementNode
  {
  public:
    LightningDeclareType(ContinueNode, TypeCopyMode::ReferenceType);
    LightningClonableNode(ContinueNode);

    // Constructor
    ContinueNode();

    // The instruction index for where the jump occurs
    size_t InstructionIndex;

    // The jump opcode that's associated with our continue (where we jump to)
    // We need to store this so that, after we build code for a function, we can
    // come back to this statement and setup the jump to the end of the loop
    RelativeJumpOpcode* JumpOpcode;
  };

  // Represents throwing an exception in langugae
  class PlasmaShared ThrowNode : public StatementNode
  {
  public:
    LightningDeclareType(ThrowNode, TypeCopyMode::ReferenceType);
    LightningClonableNode(ThrowNode);

    // Constructor
    ThrowNode();
    
    // SyntaxNode interface
    void PopulateChildren(NodeChildren& childrenOut) override;

    // The exception to be thrown
    ExpressionNode* Exception;
  };

  // A loop scope is a scope that we can break out of or continue from
  class PlasmaShared LoopScopeNode : public ScopeNode
  {
  public:
    LightningDeclareType(LoopScopeNode, TypeCopyMode::ReferenceType);

    // Default constructor
    LoopScopeNode();

    // Copy constructor
    LoopScopeNode(const LoopScopeNode& toCopy);

    // Store a list of any break statements that are targeted at us
    Array<BreakNode*> Breaks;

    // Store a list of any continue statements that are targeted at us
    Array<ContinueNode*> Continues;
  };

  // A loop that Contains a conditional expression
  class PlasmaShared ConditionalLoopNode : public LoopScopeNode
  {
  public:
    LightningDeclareType(ConditionalLoopNode, TypeCopyMode::ReferenceType);

    // Constructor
    ConditionalLoopNode();
    
    // SyntaxNode interface
    void PopulateChildren(NodeChildren& childrenOut) override;

    // The conditional expression used in this if statement
    ExpressionNode* Condition;
  };

  // A while node represents the a while loop
  class PlasmaShared WhileNode : public ConditionalLoopNode
  {
  public:
    LightningDeclareType(WhileNode, TypeCopyMode::ReferenceType);
    LightningClonableNode(WhileNode);
  };

  // A do-while node represents the a do-while loop
  class PlasmaShared DoWhileNode : public ConditionalLoopNode
  {
  public:
    LightningDeclareType(DoWhileNode, TypeCopyMode::ReferenceType);
    LightningClonableNode(DoWhileNode);
  };

  // A for node represents the a for loop
  class PlasmaShared ForNode : public ConditionalLoopNode
  {
  public:
    LightningDeclareType(ForNode, TypeCopyMode::ReferenceType);
    LightningClonableNode(ForNode);

    // Constructor
    ForNode();
    
    // SyntaxNode interface
    void PopulateChildren(NodeChildren& childrenOut) override;

    // The creation of the variable (the first part)
    LocalVariableNode* ValueVariable;

    // Only used in the case of 'foreach' to store a temporary range variable
    LocalVariableNode* RangeVariable;

    // Alternative, instead of a variable we could have an initialization expression
    ExpressionNode* Initialization;

    // The iterator expression of the for loop (the last part)
    ExpressionNode* Iterator;
  };

  // A for node represents the a for loop
  class PlasmaShared ForEachNode : public ForNode
  {
  public:
    LightningDeclareType(ForEachNode, TypeCopyMode::ReferenceType);
    LightningClonableNode(ForEachNode);

    // Constructor
    ForEachNode();
    
    // SyntaxNode interface
    void PopulateNonTraversedChildren(NodeChildren& childrenOut) override;
    
    // The original variable that was declared
    // This is not used by the Syntaxer or CodeGenerator (only there for translation and other purposes)
    LocalVariableNode* NonTraversedVariable;

    // The original range we used (eg, array.All)
    // This is not used by the Syntaxer or CodeGenerator (only there for translation and other purposes)
    ExpressionNode* NonTraversedRange;
  };

  // A loop node represents the a loop
  class PlasmaShared LoopNode : public LoopScopeNode
  {
  public:
    LightningDeclareType(LoopNode, TypeCopyMode::ReferenceType);
    LightningClonableNode(LoopNode);
  };

  // A generic function only takes parameters, has no returns and is not marked as static
  class PlasmaShared GenericFunctionNode : public ScopeNode
  {
  public:
    LightningDeclareType(GenericFunctionNode, TypeCopyMode::ReferenceType);

    // Constructor
    GenericFunctionNode();
    
    // SyntaxNode interface
    void PopulateChildren(NodeChildren& childrenOut) override;

    // The name of the function (including 'constructor', 'destructor', 'get', and 'set')
    UserToken Name;

    // A genetated type for this function (the type is the signature type)
    DelegateType* Type;

    // The function definition that this node represents (will be filled in later)
    Function* DefinedFunction;

    // The parameters defined for the function (names, types, defaults, etc)
    NodeList<ParameterNode> Parameters;

    // A pointer to the attributes this function has
    NodeList<AttributeNode> Attributes;

    // For auto-complete, one of the methods we use is to build a psuedo class and function
    // that we evaluate expressions within. This works for most expressions, except when the
    // expression relies upon the 'this' variable, in which the type would result in the pseudo class
    // Therefore, we actually replace the type with the old previously compiled version if it exists
    //BoundType* SubstituteTypeOfThisVariable;
  };

  // A function node represent the definition of a function
  class PlasmaShared FunctionNode : public GenericFunctionNode
  {
  public:
    LightningDeclareType(FunctionNode, TypeCopyMode::ReferenceType);
    LightningClonableNode(FunctionNode);

    // Constructor
    FunctionNode();
    
    // SyntaxNode interface
    String ToString() const override;
    void PopulateChildren(NodeChildren& childrenOut) override;

    // The return type of the function (or null if there is none)
    SyntaxType* ReturnType;

    // If this is an extension method, this will be the new owner of the method
    BoundSyntaxType* ExtensionOwner;

    // Is the function a static function?
    bool IsStatic;

    // Is the function a virtual function (or overriding)?
    VirtualMode::Enum Virtualized;
  };

  // Note that represents an initializer in the initializer list
  class PlasmaShared InitializerNode : public ExpressionNode
  {
  public:
    LightningDeclareType(InitializerNode, TypeCopyMode::ReferenceType);
    LightningClonableNode(InitializerNode);

    // Constructor
    InitializerNode();
    
    // Whatever it is we're initializing (this or base)
    const UserToken* InitializerType;

    // The function that this initializer invoke
    Function* InitializerFunction;
  };

  // A constructor is a specialized function for creating and initializing an object
  class PlasmaShared ConstructorNode : public GenericFunctionNode
  {
  public:
    LightningDeclareType(ConstructorNode, TypeCopyMode::ReferenceType);
    LightningClonableNode(ConstructorNode);

    // Constructor
    ConstructorNode();

    // These are not owned initializers (technically the first statements in the constructor own them)
    // Hence we do not override 'PopulateChildren' and output these
    // If the initializers exist as the first statements, these MUST be set to be a valid tree
    // The presence of the base initializer tells us if we initialized our base or not
    InitializerNode* BaseInitializer;
    InitializerNode* ThisInitializer;
  };

  // A destructor is a specialized function for destroying an object
  class PlasmaShared DestructorNode : public GenericFunctionNode
  {
  public:
    LightningDeclareType(DestructorNode, TypeCopyMode::ReferenceType);
    LightningClonableNode(DestructorNode);
  };

  // A class node represents the definition of a class
  // The class is a scope node itself because of the pre-constructor and member variable initialization
  class PlasmaShared ClassNode : public ScopeNode
  {
  public:
    LightningDeclareType(ClassNode, TypeCopyMode::ReferenceType);
    LightningClonableNode(ClassNode);

    // Constructor
    ClassNode();
    
    // SyntaxNode interface
    String ToString() const override;
    void PopulateChildren(NodeChildren& childrenOut) override;

    // The name of the class
    UserToken Name;

    // If the node represents a value type or not
    TypeCopyMode::Enum CopyMode;

    // The resolved type of the class
    BoundType* Type;

    // The names of the parent types
    SyntaxTypeList Inheritance;

    // Any template arguments
    Array<const UserToken*> TemplateArguments;

    // A list of member variables defined in the class
    NodeList<MemberVariableNode> Variables;

    // A list of functions defined in the class
    NodeList<FunctionNode> Functions;

    // A list of constructors defined in the class
    NodeList<ConstructorNode> Constructors;

    // A list of events that we send
    NodeList<SendsEventNode> SendsEvents;

    // A singular destructor
    DestructorNode* Destructor;

    // Contains all types of members that a class can have in the order they are declared (generally used for formatting)
    NodeList<SyntaxNode> NonTraversedNonOwnedNodesInOrder;

    // This function basically acts as a constructor that initializes all the members before we run the invoked constructor
    Function* PreConstructor;

    // A pointer to the attributes this class has
    NodeList<AttributeNode> Attributes;

    // Only valid when the class is a templated class
    // This is used for when the syntax tree for the class gets cloned
    // in order to make a template instantiation
    // (this is how we know it is a clone and not the original source!)
    const BoundSyntaxType* TemplateInstantiation;

    // Check if this class is a templated class
    bool IsTemplate() const;
  };

  // An enum value declared within an enum
  class PlasmaShared EnumValueNode : public SyntaxNode
  {
  public:
    LightningDeclareType(EnumValueNode, TypeCopyMode::ReferenceType);
    LightningClonableNode(EnumValueNode);

    // Constructor
    EnumValueNode();

    // SyntaxNode interface
    String ToString() const override;

    // The name of the value
    UserToken Name;
    
    // The integral value of this entry (or null if there is no user set value)
    const UserToken* Value;

    // The actual value assigned to this enum entry (Syntaxer)
    Integer IntegralValue;

    // The static property that we use at runtime to get the value
    Property* IntegralProperty;
  };

  // An enum node represents constant integral values that count up (or bitwise flags)
  class PlasmaShared EnumNode : public SyntaxNode
  {
  public:
    LightningDeclareType(EnumNode, TypeCopyMode::ReferenceType);
    LightningClonableNode(EnumNode);

    // Constructor
    EnumNode();
    
    // SyntaxNode interface
    String ToString() const override;
    void PopulateChildren(NodeChildren& childrenOut) override;

    // The name of the enum
    UserToken Name;

    // Whether or not this enum is considered to be flags
    // Flags allow operations such as bitwise or/and/xor
    bool IsFlags;

    // The resolved type of the enum
    BoundType* Type;

    // The names of the parent type, or null if we don't have one
    SyntaxType* Inheritance;

    // A list of enum values (integral constants)
    // These values may have a user set value or may be auto-picked
    NodeList<EnumValueNode> Values;

    // A pointer to the attributes this class has
    NodeList<AttributeNode> Attributes;
  };

  // A type-define node represents 
  class PlasmaShared TypeDefineNode : public SyntaxNode
  {
  public:
    LightningDeclareType(TypeDefineNode, TypeCopyMode::ReferenceType);
    LightningClonableNode(TypeDefineNode);

    // Constructor
    TypeDefineNode();

    // SyntaxNode interface
    String ToString() const override;
    void PopulateChildren(NodeChildren& childrenOut) override;

    // The name that we're giving the type-definition
    const UserToken* Name;

    // The type that we represent
    SyntaxType* Type;
  };

  // A local variable reference node replaces a generic identifier node
  class PlasmaShared LocalVariableReferenceNode : public ValueNode
  {
  public:
    LightningDeclareType(LocalVariableReferenceNode, TypeCopyMode::ReferenceType);
    LightningClonableNode(LocalVariableReferenceNode);

    // Constructor
    LocalVariableReferenceNode();

    // SyntaxNode interface
    String ToString() const override;

    // Store a reference to the variable
    Variable* AccessedVariable;
  };
}

#endif