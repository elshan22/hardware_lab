from rest_framework import views, status
from rest_framework.permissions import AllowAny
from rest_framework.response import Response
from rest_framework.views import APIView

from .serializers import AuthenticationSerializer, LogoutSerializer
import paho.mqtt.client as mqtt
from rest_framework_simplejwt.views import TokenObtainPairView


# def on_connect(client, userdata, flags, rc):
#     print("Connected with result code", rc)
#     client.subscribe("your_topic")
#
#
# def on_message(client, userdata, msg):
#     print("Received message:", msg.payload.decode('utf-8'))
#
#
# client = mqtt.Client()
# client.on_connect = on_connect
# client.on_message = on_message
# client.connect("1.1.1.1", 1700, 60)
# client.loop_forever()


class LoginView(TokenObtainPairView):
    serializer_class = AuthenticationSerializer


class RegisterView(APIView):
    serializer_class = AuthenticationSerializer
    permission_classes = [AllowAny]

    def post(self, request):
        serializer = self.serializer_class(data=request.data)
        if serializer.is_valid():
            user = serializer.save()
            if user:
                json = serializer.data
                return Response(json, status=status.HTTP_201_CREATED)
        return Response(serializer.errors, status=status.HTTP_400_BAD_REQUEST)


class LogoutView(APIView):
    serializer_class = LogoutSerializer
    permission_classes = [AllowAny]

    def post(self, request):
        serializer = self.serializer_class(data=request.data)
        serializer.is_valid(raise_exception=True)
        serializer.save()
        return Response(status=status.HTTP_204_NO_CONTENT)
