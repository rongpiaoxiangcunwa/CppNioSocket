#!/usr/bin/python

import threading
import signal
import os
import time
import Queue
import sys

class ComponentCompileTime:
	def __init__(self):
		self.componentTime = {}
	def addCompileTime( self, component, time ):
		self.componentTime[component] = time

	def printCompileTime( self ):
		for component in sorted( self.componentTime, key=self.componentTime.get, reverse= True):
			print "compile time of %s:%d" % (component, self.componentTime[component] )

class Components:
	def __init__( self, config, excludeComponents ):
		self.components = {}
		self.config = config
		self.excludeComponents = excludeComponents
		self._loadComponents('Makefile')
		# remove the component from excludeComponents if it must be compiled
        	for component in self.components:
            		for depend in self.components[component]['depends']:
                        	if depend in self.excludeComponents:
                			self.excludeComponents.remove( depend )
	def getComponentNames(self):
		return list( set( self.components.keys() ) - set( self.excludeComponents ) )

	def getComponentNamesExceptFor( self, componentNames ):
		result = []
		for component in self.components:
			if component in componentNames:
				pass
			else:
				result.append( component )
		return result

	def getConfig( self ):
		return self.config

	def printDepends( self ):
		for component in self.components:
			print( '%s:%s'% (component, ' '.join( self.components[component]['depends'] ) ) )
	def getComponentDepends( self, componentNames, recursive):
		depends = set()
		for component in componentNames:
			if component in self.components:
				directDepends = self.components[component]['depends']
				depends = depends.union( set( directDepends ) )
				if recursive and directDepends:	
					depends = depends.union( self.getComponentDepends( directDepends, recursive ) )
		return depends

	def getComponentsWithSolvedDepends( self, compiledComponents ):
		result=[]
		for component in self.components:
			if self._isComponentDependsSolved( component, compiledComponents ):
				result.append( component )
		return result

	def genMakeForEachObj( self ):
		result=[]
		for component in self.components:
			for obj in self.components[component]['objs']:
				result.append( 'make config=%s -f %s.make %s/%s' % (self.config, component, self.components[component]['objDir'], obj ) )
		return result

	def getAppsExceptFor( self, apps ):
		result = []
		for component in self.components:
			if self.components[component]['componentType'] == 'app' and not component in apps:
				result.append( component )
		return result

	def _isComponentDependsSolved( self, component, compiledComponents ):
		for depend in self.components[component]['depends']:
			if not depend in compiledComponents:
				return False
		return True
		
	def _loadComponents( self, makeFile ):
		f = open( makeFile )
        	projects = "PROJECTS := "
		componentNames = []
        	for line in f:
                	if line[0:len(projects)] == projects:
                        	componentNames = line[len(projects):].split()
        	f.close()

		for componentName in componentNames:
			self._loadComponentInfo( componentName, componentNames )	

	def _loadComponentInfo( self, componentName, allComponentNames ):
		f = open( '%s.make' % componentName )
        	configFound = False
        	componentType = ''
        	depends = []
		objDir = ''
		objs = []
        	for line in f:
                	if not configFound and line.find( "ifeq ($(config)," + self.config + ")" ) == 0:
                        	configFound = True
                	elif configFound and line.find( '  LIBS      += ' ) == 0:
                        	for lib in line[len('  LIBS      += '):].split():
                                	if lib.find( '-l' ) == 0:
						dependComponent = lib[2:]
						if dependComponent in allComponentNames and not dependComponent in depends:
                                        		depends.append( dependComponent )
                	elif configFound and line.find( '  TARGET     = ' ) == -1:
                        	if line.strip()[-3:] == '.so' or line.strip()[-2:] == '.a':
                                	componentType = 'lib'
                        	else:
                                	componentType = 'app'
			elif configFound and line.find( '  OBJDIR     = ' ) == 0 and not objDir:
				objDir = line[len('  OBJDIR     = '):].strip()
			elif configFound and line.find( '$(OBJDIR)/' ) == 0:
				objAndSrc = line[len('$(OBJDIR)/'):].split()
				if len( objAndSrc ) == 2 and objAndSrc[0][-1:] == ':':
					objs.append( objAndSrc[0][:-1] )
		self.components[componentName]={'depends':depends,'componentType':componentType, 'objDir':objDir, 'objs': objs }
	

class CompileScheduler:
	def __init__( self, components, options, toBeCompiledComponents, compileThreads ):
		self.components = components
		self.options = options

		if toBeCompiledComponents:
			self.toBeCompiledComponents = set( toBeCompiledComponents ).union( set( self.components.getComponentDepends( toBeCompiledComponents, True ) ) ).intersection( self.components.getComponentNames() )
		else:
			self.toBeCompiledComponents = self.components.getComponentNames()
                #print self.toBeCompiledComponents
		self.compileThreads = compileThreads
                print( 'threads: %d' % self.compileThreads )
		self.compilationState = {}
		self.pendingComponents = Queue.Queue()
		self.highPriorityCompileComponents = self.components.getComponentDepends( [], True )
		self.componentCompileTime = ComponentCompileTime()
		self.lock = threading.Lock()

	def start( self ):
		
		# init the state, if the component will not be compiled, set its state to 'Finished' otherwise
		# set its state to 'NotStart'	
		for component in self.components.getComponentNames():
			if component in self.toBeCompiledComponents:
				self.compilationState[component] = 'NotStart'
			else:
				self.compilationState[component] = 'Finished'


		componentsWithOutDepends = set( self.toBeCompiledComponents ).intersection( set( self.components.getComponentsWithSolvedDepends([]) ) )
		self._addToPendingQueue( componentsWithOutDepends )

		self._startCompileThreads()	
		self.componentCompileTime.printCompileTime()

	def _addToPendingQueue( self, components ):
		for component in components:
			if component in self.highPriorityCompileComponents and self.compilationState[component] == 'NotStart':
				self.compilationState[component] = 'Started'
                                self.pendingComponents.put( component )
		for component in components:
			if not component in self.highPriorityCompileComponents and self.compilationState[component] == 'NotStart':
				self.compilationState[component] = 'Started'
                                self.pendingComponents.put( component )

	def _startCompileThreads( self ):	
		threads = []
		for i in range(0,self.compileThreads):
			t = threading.Thread( target = self._compile )
			threads.append( t )
			t.start()
		#wait for compile thread exit
		for t in threads:
			t.join()

	def _getCompileJobs( self, component ):

		if component in ['sctp_server']:
			if os.environ.has_key('MAKE_JOBS'):
				return int (os.environ.get('MAKE_JOBS') ) 
			return 4
		else:
			if os.environ.has_key('MAKE_JOBS'):
				return int (os.environ.get('MAKE_JOBS') ) 
			return 1
	def _compile( self ):
		while True:
			with self.lock:
				if self._isAllComponentsFinished():
					return
			component = ''
			try:
				component = self.pendingComponents.get(True, 1 )
		                #print self.compilationState
				start_time = time.time()
				with self.lock:
					print('==== Building %s (%s) ===='%( component, self.components.getConfig() ) )
				exitCode = os.system( 'make -j %d %s --no-print-directory -C . -f %s.make' % ( self._getCompileJobs( component ), ' '.join(self.options), component ) )
				with self.lock:
					#print( 'make -j %d %s --no-print-directory -C . -f %s.make' % ( self._getCompileJobs( component ), ' '.join(self.options), component ) )
					#print( 'Compile %s' % component )
					self._finishCompile( component )
					self.componentCompileTime.addCompileTime( component, time.time() - start_time )
				if exitCode != 0:
                                        os._exit(-1)
			except Exception as err:
				time.sleep( 1 )

	def _finishCompile( self, component ):
		self.compilationState[component] = 'Finished'

		#change the state to Finished
		finishedComponents = []
		for p in self.compilationState:
			if self.compilationState[ p ] == 'Finished':
				finishedComponents.append( p )


		solvedDependsComponents = set( self.components.getComponentsWithSolvedDepends( finishedComponents ) ).intersection( set( self.toBeCompiledComponents ) )
		self._addToPendingQueue( solvedDependsComponents )
				
	def _isAllComponentsFinished( self ):
		for component in self.compilationState:
			if self.compilationState[ component ] != 'Finished':
				return False
		return True

def getComponentsWithGroup( components, groupComponents ):
	result = []
	for component in components:
		if component in groupComponents:
			result.extend( groupComponents[component] )
		else:
			result.append( component )
	return result
def parseArguments():
	"""
		parse the arguments from command line and return a tuple:
			- options: transpently passed to the make
			- components: in the command line will be compiled
			- config: if the config parameter is in the command line
			- excludeComponents: the components should not be compiled
	"""
	options = []
	components = []
	hasConfig=False
	excludeComponents = []
	groupComponents = None
	for i in range( 1, len( sys.argv ) ):
		arg = sys.argv[i]
		if arg.startswith( "--excludeFile="):
                        excludeComponents.extend( loadExcludeComponentsFromFile(arg[len("--excludeFile="):]) )
                elif arg.startswith( "--excludeComponents="):
                        excludeComponents.extend( parseExcludeComponents( arg[len("--excludeComponents="):]) )
		elif arg.startswith( "--group=" ):
			groupComponents = loadGroup( arg[len( "--group=" ):] )
		elif arg.find( '=' ) > 0:
			options.append( arg )
			if arg.find( 'config=' ) == 0:
				hasConfig = True
		else:
			if arg[-5:] == ".make":
				components.append( arg[0:len(arg)-5] )
			else:
				components.append( arg )

	if not hasConfig:
		options.append( 'config=debug64' )
	if groupComponents:
		excludeComponents = getComponentsWithGroup( excludeComponents, groupComponents )
		components = getComponentsWithGroup( components, groupComponents )
	return (options,components, list( set( excludeComponents) - set( components ) ) )

def loadGroup( fileName ):
	groupComponents = {}
	with open( fileName ) as fp:
		group = ""
		for line in fp:
			line = line.strip()
			if line.startswith( '#' ):
				continue
			elif line.startswith( '[') and line.endswith( ']'):
				group = line[1:-1].strip()
				groupComponents[group] = []
			elif group:
				for component in line.split( "," ):
					groupComponents[group].append( component.strip() )
	return groupComponents
				
def loadExcludeComponentsFromFile( fileName ):
	components = []
	with open(fileName) as f:
		for line in f:
			for component in line.split( "," ):
				components.append( component.strip() )
	return components

def parseExcludeComponents( excludeComponents ):
	components = []
	for component in excludeComponents.split( "," ):
		components.append( component.strip() )
	return components

def getConfig( options ):
	for option in options:
		i = option.find( '=' )
		name = option[0:i]
		if name == 'config':
			return option[i+1:]
	return 'debug64'

def main():
	options, toBeCompiledComponents, excludeComponents = parseArguments()
	components = Components(getConfig(options), excludeComponents)
	if 'clean' in toBeCompiledComponents:
		os.system( 'make config=%s clean' % getConfig( options ) )
	#components.printDepends()

	#print( "components number=%d" % ( len( components.getComponentNames() ) ) )
	#print components.getComponentNames()
	task_num = os.getenv ('MAKE_JOBS', '4' )
	start_time = time.time()
	compileScheduler = CompileScheduler( components, options, toBeCompiledComponents, int(task_num) )
	compileScheduler.start()

	end_time = time.time()
	total_time = end_time - start_time
	print( 'total time:%d' % total_time )

main()
