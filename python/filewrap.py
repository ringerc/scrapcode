"""
This Python module wraps a `file' object, allowing you to add your own
hooks for, say, tracking progress of a read or write. An example that tracks
and reports reads is implemented.
"""

class FileWrapper(object):

	def __init__(self, wrapped):
		"""Wrap a file object with progress-tracking information"""
		self._wrapped = wrapped
	
	def __getattr__(self,attr):
		"""Auto-wrap methods not explicitly defined"""
		orig_attr = self._wrapped.__getattribute__(attr)
		if callable(orig_attr):
			def hooked(*args, **kwargs):
				result = orig_attr(*args, **kwargs)
				return result
			return hooked
		else:
			return orig_attr

	def read(self, *args):
		read_hook(*args)
		return self._wrapped.read(*args)

	def read_hook(self, *args):
		"""Override this to track read progress"""
		pass
