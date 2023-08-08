from rest_framework import views, status
from rest_framework.response import Response
from .models import Device, DeviceState
from .serializers import DeviceStateSerializer

class DeviceStateView(views.APIView):
    def post(self, request, device_id):
        try:
            device = Device.objects.get(pk=device_id)
        except Device.DoesNotExist:
            return Response(status=status.HTTP_404_NOT_FOUND)

        new_state = request.data.get('state', None)
        if new_state is None:
            return Response(status=status.HTTP_400_BAD_REQUEST)

        DeviceState.objects.create(device=device, state=new_state)
        return Response(status=status.HTTP_201_CREATED)

    def get(self, request, device_id):
        try:
            device = Device.objects.get(pk=device_id)
        except Device.DoesNotExist:
            return Response(status=status.HTTP_404_NOT_FOUND)

        latest_state = DeviceState.objects.filter(device=device).latest('timestamp')
        serializer = DeviceStateSerializer(latest_state)
        return Response(serializer.data)
