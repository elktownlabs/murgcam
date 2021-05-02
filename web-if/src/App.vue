<template>
  <v-app>
    <v-app-bar dense app v-if="this.$store.getters.isAuthenticated">
      <v-app-bar-title><v-icon class="mx-3">mdi-camera-wireless</v-icon>Remote Webcam</v-app-bar-title>
      <v-spacer/>
      <v-btn to="/settings" class="mx-1" dark color="indigo"><v-icon>mdi-cogs</v-icon><span class="d-none d-md-block">Settings</span></v-btn>      
      <v-btn @click="logout" class="mx-1" dark color="indigo"><v-icon>mdi-logout</v-icon><span class="d-none d-md-block">Logout</span></v-btn>
    </v-app-bar>
    <v-main>
      <router-view></router-view>
    </v-main>
  </v-app>
</template>

<script>
import { AUTH_LOGOUT } from "./store/actions/auth";

export default {
  name: 'App',
  mounted() {
    document.title = "WebCam Admin Interface"
  },
  methods: {
    logout() {
      this.$store.dispatch(AUTH_LOGOUT).then(() => {
        this.$router.push('/login')
      }).catch(() => { /* TODO */ });
    }
  }
}
</script>