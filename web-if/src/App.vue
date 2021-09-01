<template>
  <v-app>
    <v-app-bar dense app v-if="this.$store.getters.isAuthenticated">
      <v-app-bar-title><v-icon class="mx-3">mdi-camera-wireless</v-icon>MurgCam</v-app-bar-title>
      <v-spacer/>
      <v-btn to="/photos" class="mx-1 hidden-sm-and-down" dark color="primary"><v-icon>mdi-camera</v-icon><span>Photos</span></v-btn>      
      <v-btn v-if="this.$store.getters.hasRight('freq')" to="/frequency" class="mx-1 hidden-sm-and-down" dark color="primary"><v-icon>mdi-clock-fast</v-icon><span>Photo Freq.</span></v-btn>
      <v-btn v-if="this.$store.getters.hasRight('set')" to="/settings" class="mx-1 hidden-sm-and-down" dark color="primary"><v-icon>mdi-cogs</v-icon><span>Settings</span></v-btn>
      <div class="mx-3"></div>
      <v-menu transition="slide-y-transition" bottom>
        <template v-slot:activator="{ on, attrs }">
            <v-btn class="mx-1 hidden-sm-and-down" icon v-bind="attrs" v-on="on">
              <v-avatar color="secondary lighten-2 white--text" size="36">
                <v-img v-if="$store.getters.currentUserAvatar != null" :src="$store.getters.currentUserAvatar" />
                <div v-if="$store.getters.currentUserAvatar == null">{{ $store.getters.currentUserInitials }}</div>
              </v-avatar>
            </v-btn>
        </template>
        <v-card elevation="2" outlined class="mx-auto" color="light-blue lighten-4">
          <v-list-item>
            <v-list-item-content>
              <v-list-item-title class="text-h6 mb-1">{{ this.$store.getters.currentUserFullName }}</v-list-item-title>
              <v-list-item-subtitle>({{ this.$store.getters.currentUser }})</v-list-item-subtitle>
            </v-list-item-content>
            <v-list-item-avatar size="60" color="grey">
              <v-img v-if="this.$store.getters.currentUserAvatar != null" :src="this.$store.getters.currentUserAvatar" />
              <div v-if="this.$store.getters.currentUserAvatar == null">{{ this.$store.getters.currentUserInitials }}</div>
            </v-list-item-avatar>
          </v-list-item>
          <v-card-actions>
            <v-btn outlined rounded @click="logout">
              <v-icon class="mx-1">mdi-logout</v-icon>Logout
            </v-btn>
          </v-card-actions>

        </v-card>
      </v-menu>
      <v-menu transition="slide-y-transition" bottom>
        <template v-slot:activator="{ on, attrs }">
            <v-btn class="mx-1 hidden-md-and-up" icon v-bind="attrs" v-on="on">
              <v-icon>mdi-menu</v-icon>
            </v-btn>
        </template>
        <v-list>
          <v-list-item class="primary lighten-2">
            <v-list-item-avatar>
              <v-img v-if="this.$store.getters.currentUserAvatar != null" :src="this.$store.getters.currentUserAvatar" />
              <div v-if="this.$store.getters.currentUserAvatar == null">{{ this.$store.getters.currentUserInitials }}</div>
            </v-list-item-avatar>
            <v-list-item-content>
              <v-list-item-title>{{ this.$store.getters.currentUserFullName }}</v-list-item-title>
              <v-list-item-subtitle>({{ this.$store.getters.currentUser }})</v-list-item-subtitle>
            </v-list-item-content>
          </v-list-item>
          <v-divider></v-divider>
          <v-list-item to="/photos">
            <v-list-item-icon><v-icon>mdi-camera</v-icon></v-list-item-icon>
            <v-list-item-content>
              <v-list-item-title>Photos</v-list-item-title>
              <v-list-item-subtitle>Library of taken photos</v-list-item-subtitle>
            </v-list-item-content>
          </v-list-item>
          <v-list-item to="/frequency">
            <v-list-item-icon><v-icon>mdi-clock-fast</v-icon></v-list-item-icon>
            <v-list-item-content>
              <v-list-item-title>Photo Frequency</v-list-item-title>
              <v-list-item-subtitle>Temporarily increase photo frequency</v-list-item-subtitle>
            </v-list-item-content>
          </v-list-item>
          <v-list-item to="/settings">
            <v-list-item-icon><v-icon>mdi-cogs</v-icon></v-list-item-icon>
            <v-list-item-content>
              <v-list-item-title>Camera Settings</v-list-item-title>
              <v-list-item-subtitle>Change operational settings</v-list-item-subtitle>
            </v-list-item-content>
          </v-list-item>
          <v-list-item @click="logout">
            <v-list-item-icon><v-icon>mdi-logout</v-icon></v-list-item-icon>
            <v-list-item-content>
              <v-list-item-title>Logout</v-list-item-title>
              <v-list-item-subtitle>Quit session</v-list-item-subtitle>
             </v-list-item-content>
          </v-list-item>
        </v-list>
      </v-menu>

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