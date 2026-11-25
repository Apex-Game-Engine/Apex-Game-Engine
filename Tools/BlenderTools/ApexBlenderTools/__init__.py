from . import operators

def register():
	operators.register()

def unregister():
	operators.unregister()

if __name__ == '__main__':
	register()
