import grpc


class TokenAuthPlugin(grpc.AuthMetadataPlugin):
    def __init__(self, initial_token: str) -> None:
        self._token = initial_token
        super().__init__()

    def __call__(self, context, callback):
        try:
            metadata = (("authorization", self._token),)

            callback(metadata, None)

        except Exception as e:
            callback((), e)
