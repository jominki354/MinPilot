from common.params import Params

try:
    from common.params_pyx import UnknownKeyName
except ImportError:
    # Fallback for environments where params_pyx cannot be imported directly or differs
    class UnknownKeyName(Exception):
        pass


class SafeParams:
    def __init__(self):
        self.params = Params()

    def get_bool(self, key, default=False):
        """
        Safely retrieve a boolean parameter.
        Returns the value if the key exists, otherwise returns the default value.
        Catches UnknownKeyName to prevent crashes on missing keys.
        """
        try:
            return self.params.get_bool(key)
        except (UnknownKeyName, Exception):
            return default

    def get(self, key, encoding=None, default=None):
        """
        Safely retrieve a string/bytes parameter.
        Returns the value if the key exists, otherwise returns the default value.
        Catches UnknownKeyName to prevent crashes on missing keys.
        """
        try:
            val = self.params.get(key, encoding=encoding)
            if val is None:
                return default
            return val
        except (UnknownKeyName, Exception):
            return default

    def put(self, key, value):
        """
        Passthrough to Params.put, but could be safe-guarded if needed.
        For now, we assume put is generally safe or specific errors are handled by caller.
        """
        self.params.put(key, value)

    def put_bool(self, key, value):
        """
        Passthrough to Params.put_bool.
        """
        self.params.put_bool(key, value)
