from tokenize import TokenError

from rest_framework import serializers
from rest_framework_simplejwt.serializers import TokenObtainPairSerializer
from rest_framework_simplejwt.tokens import RefreshToken


class AuthenticationSerializer(TokenObtainPairSerializer):
    @classmethod
    def get_token(cls, user):
        token = super().get_token(user)

        token['username'] = user.username
        token['password'] = user.password
        token['email'] = user.email
        token['is_admin'] = user.is_admin

        return token


class LogoutSerializer(serializers.Serializer):
    refresh = serializers.CharField()

    def __init__(self, data):
        super().__init__(data)
        self.token = None

    default_error_message = {
        'bad_token': 'Token is expired or invalid'
    }

    def validate(self, attrs):
        self.token = attrs['refresh']
        return attrs

    def save(self, **kwargs):
        try:
            RefreshToken(self.token).blacklist()
        except TokenError:
            self.fail('bad_token')
